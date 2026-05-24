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
            OrderId id = o->id;
            Order* ptr = o.get();
            order_store[id] = std::move(o);
            book.add_order(ptr);
        } else {
            // Market orders do not rest
            // In a real system, we might emit a Cancel event here.
            // For now, the unique_ptr will go out of scope and free the order.
        }
    }
    // 5. If fully filled, unique_ptr goes out of scope and deletes the order.
}

void MatchingEngine::cancel_order(OrderId id) {
    auto it = order_store.find(id);
    if (it == order_store.end()) {
        return; // Silently ignore if not found
    }

    Order* o = it->second.get();
    auto book_it = books.find(o->symbol);
    if (book_it != books.end()) {
        book_it->second.remove_order(o);
    }

    order_store.erase(it);
}

void MatchingEngine::reduce_order(OrderId id, Qty cancelled_qty) {
    auto it = order_store.find(id);
    if (it == order_store.end()) {
        return;
    }

    Order* o = it->second.get();
    if (cancelled_qty >= o->quantity) {
        cancel_order(id);
    } else {
        o->quantity -= cancelled_qty;
        // No need to update the book/price level structure because:
        // 1. Price is unchanged.
        // 2. Position in list (time priority) is unchanged.
        // 3. PriceLevel::total_quantity() computes on demand.
    }
}

void MatchingEngine::replace_order(OrderId old_id, OrderId new_id, Price new_price, Qty new_qty, Timestamp new_ts) {
    auto it = order_store.find(old_id);
    if (it == order_store.end()) {
        return;
    }

    // Capture metadata from old order
    Symbol symbol = it->second->symbol;
    Side side = it->second->side;
    
    cancel_order(old_id);
    
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

        // Perform the trade
        Order* passive = level->front();
        Qty fill_qty = std::min(aggressive->quantity, passive->quantity);

        aggressive->quantity -= fill_qty;
        passive->quantity -= fill_qty;

        if (on_fill) {
            on_fill({aggressive->id, passive->id, passive->price, fill_qty, aggressive->timestamp});
        }

        if (passive->quantity == 0) {
            OrderId passive_id = passive->id;
            book.remove_order(passive);
            order_store.erase(passive_id);
        }
    }
}

} // namespace engine
