#include "matching_engine.hpp"
#include <cassert>

namespace engine {

void MatchingEngine::set_on_fill(FillCallback cb) {
    on_fill = std::move(cb);
}

void MatchingEngine::submit_order(std::unique_ptr<Order> o) {
    // 1. Validate
    assert(o->quantity > 0);
    if (o->type == OrderType::Limit) {
        assert(o->price > 0);
    }

    // 2. Look up or create the OrderBook
    auto& book = books[o->symbol];

    // 3. Run the matching loop
    match(o.get(), book);

    // 4. Handle remaining quantity
    if (o->quantity > 0) {
        if (o->type == OrderType::Limit) {
            // Allocate from pool and copy the transient order data
            Order* pooled_order = order_pool.allocate(
                o->id, o->symbol, o->side, o->type, o->price, o->quantity, o->timestamp
            );
            
            order_store[pooled_order->id] = pooled_order;
            book.add_order(pooled_order);
        }
    }
}

void MatchingEngine::cancel_order(OrderId id) {
    auto it = order_store.find(id);
    if (it == order_store.end()) {
        return; 
    }

    Order* o = it->second;
    auto book_it = books.find(o->symbol);
    if (book_it != books.end()) {
        book_it->second.remove_order(o);
    }

    order_pool.deallocate(o);
    order_store.erase(it);
}

void MatchingEngine::reduce_order(OrderId id, Qty cancelled_qty) {
    auto it = order_store.find(id);
    if (it == order_store.end()) {
        return;
    }

    Order* o = it->second;
    if (cancelled_qty >= o->quantity) {
        // Inline cancellation to avoid double lookup
        auto book_it = books.find(o->symbol);
        if (book_it != books.end()) {
            book_it->second.remove_order(o);
        }
        order_pool.deallocate(o);
        order_store.erase(it);
    } else {
        o->quantity -= cancelled_qty;
    }
}

void MatchingEngine::replace_order(OrderId old_id, OrderId new_id, Price new_price, Qty new_qty, Timestamp new_ts) {
    auto it = order_store.find(old_id);
    if (it == order_store.end()) {
        return;
    }

    Order* o = it->second;
    Symbol symbol = o->symbol;
    Side side = o->side;
    
    // Inline cancellation to avoid double lookup
    auto book_it = books.find(symbol);
    if (book_it != books.end()) {
        book_it->second.remove_order(o);
    }
    order_pool.deallocate(o);
    order_store.erase(it);
    
    auto new_order = std::make_unique<Order>(new_id, symbol, side, OrderType::Limit, new_price, new_qty, new_ts);
    submit_order(std::move(new_order));
}

void MatchingEngine::match(Order* aggressive, OrderBook& book) {
    while (aggressive->quantity > 0) {
        PriceLevel* level = nullptr;
        bool can_match = false;

        if (aggressive->side == Side::Buy) {
            level = book.best_ask();
            if (level) {
                if (aggressive->type == OrderType::Market || book.best_ask_price() <= aggressive->price) {
                    can_match = true;
                }
            }
        } else {
            level = book.best_bid();
            if (level) {
                if (aggressive->type == OrderType::Market || book.best_bid_price() >= aggressive->price) {
                    can_match = true;
                }
            }
        }

        if (!can_match) break;

        Order* passive = level->front();
        Qty fill_qty = std::min(aggressive->quantity, passive->quantity);

        aggressive->quantity -= fill_qty;
        
        // Update price level's cumulative quantity
        level->decrement_cumulative_qty(fill_qty);
        passive->quantity -= fill_qty;

        if (on_fill) {
            on_fill({aggressive->id, passive->id, passive->price, fill_qty, aggressive->timestamp});
        }

        if (passive->quantity == 0) {
            OrderId passive_id = passive->id;
            book.remove_order(passive);
            order_store.erase(passive_id);
            order_pool.deallocate(passive);
        }
    }
}

} // namespace engine
