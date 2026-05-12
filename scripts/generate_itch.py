import struct
import random
import time
from datetime import datetime, timedelta

# NASDAQ ITCH 5.0 Constants
PRICE_SCALE = 10000
TIMESTAMP_BASE = 0  # Nanoseconds since midnight

def get_nanoseconds_since_midnight():
    now = datetime.now()
    midnight = now.replace(hour=0, minute=0, second=0, microsecond=0)
    diff = now - midnight
    return int(diff.total_seconds() * 1e9)

class ITCHGenerator:
    def __init__(self, output_path):
        self.output_path = output_path
        self.order_ref_counter = 1
        self.active_orders = {}  # ref_num -> (symbol, side, qty, price)
        self.stock_locate_map = {}
        self.symbols = ["AAPL", "MSFT", "GOOG", "AMZN", "TSLA"]
        for i, sym in enumerate(self.symbols):
            self.stock_locate_map[sym] = i + 1

    def _pack_msg(self, fmt, *args):
        # Format string 'B' is for message type (1 byte), then the rest
        msg = struct.pack(fmt, *args)
        # ITCH messages in a file are usually prefixed with 2-byte big-endian length
        length = len(msg)
        return struct.pack('>H', length) + msg

    def system_event(self, event_code):
        # Type 'S', Locate (2), Tracking (2), Timestamp (6), Event (1)
        ts = get_nanoseconds_since_midnight()
        return self._pack_msg('>cHH6sc', b'S', 0, 0, ts.to_bytes(6, 'big'), event_code.encode())

    def stock_directory(self, symbol):
        # Type 'R', Locate (2), Tracking (2), Timestamp (6), Stock (8), ... (many fields)
        # We simplify the trailing fields for synthetic data
        ts = get_nanoseconds_since_midnight()
        locate = self.stock_locate_map[symbol]
        stock_padded = symbol.ljust(8).encode()
        # R (1), Locate (2), Track (2), TS (6), Stock (8), MarketCat (1), FinStatus (1), LotSize (4), ...
        # Total size excluding length: 1+2+2+6+8+1+1+4+1+1+2+1+1+1+1+1+4+1 = 39 bytes
        # Using a simplified version for the filler fields
        return self._pack_msg('>cHH6s8sccIcc2scccccIc', 
            b'R', locate, 0, ts.to_bytes(6, 'big'), stock_padded, 
            b'Q', b'N', 100, b'N', b'C', b'  ', b'P', b'N', b' ', b'1', b'N', 0, b'N'
        )

    def add_order(self, symbol, side, qty, price):
        ts = get_nanoseconds_since_midnight()
        locate = self.stock_locate_map[symbol]
        ref_num = self.order_ref_counter
        self.order_ref_counter += 1
        stock_padded = symbol.ljust(8).encode()
        
        self.active_orders[ref_num] = (symbol, side, qty, price)
        
        # Type 'A', Locate (2), Tracking (2), Timestamp (6), Ref (8), Side (1), Qty (4), Stock (8), Price (4)
        return self._pack_msg('>cHH6sQcI8sI',
            b'A', locate, 0, ts.to_bytes(6, 'big'), ref_num, side.encode(), qty, stock_padded, int(price * PRICE_SCALE)
        )

    def execute_order(self, ref_num, qty):
        if ref_num not in self.active_orders: return b''
        symbol, side, current_qty, price = self.active_orders[ref_num]
        exec_qty = min(qty, current_qty)
        
        ts = get_nanoseconds_since_midnight()
        locate = self.stock_locate_map[symbol]
        match_num = random.randint(1, 1000000)
        
        if exec_qty == current_qty:
            del self.active_orders[ref_num]
        else:
            self.active_orders[ref_num] = (symbol, side, current_qty - exec_qty, price)

        # Type 'E', Locate (2), Tracking (2), Timestamp (6), Ref (8), Qty (4), Match (8)
        return self._pack_msg('>cHH6sQIQ',
            b'E', locate, 0, ts.to_bytes(6, 'big'), ref_num, exec_qty, match_num
        )

    def delete_order(self, ref_num):
        if ref_num not in self.active_orders: return b''
        symbol, _, _, _ = self.active_orders[ref_num]
        ts = get_nanoseconds_since_midnight()
        locate = self.stock_locate_map[symbol]
        del self.active_orders[ref_num]
        
        # Type 'D', Locate (2), Tracking (2), Timestamp (6), Ref (8)
        return self._pack_msg('>cHH6sQ',
            b'D', locate, 0, ts.to_bytes(6, 'big'), ref_num
        )

    def generate(self, num_messages=1000):
        with open(self.output_path, 'wb') as f:
            # Start of Day
            f.write(self.system_event('O'))
            
            # Stock Directory
            for sym in self.symbols:
                f.write(self.stock_directory(sym))
            
            # Random Orders
            for _ in range(num_messages):
                action = random.random()
                if action < 0.6 or not self.active_orders:
                    # Add Order
                    sym = random.choice(self.symbols)
                    side = random.choice(['B', 'S'])
                    qty = random.randrange(10, 1000, 10)
                    base_price = 150 if sym == "AAPL" else 300
                    price = base_price + random.uniform(-5, 5)
                    f.write(self.add_order(sym, side, qty, price))
                elif action < 0.8:
                    # Execute
                    ref_num = random.choice(list(self.active_orders.keys()))
                    _, _, qty, _ = self.active_orders[ref_num]
                    exec_qty = random.randrange(10, qty + 10, 10)
                    f.write(self.execute_order(ref_num, exec_qty))
                else:
                    # Delete
                    ref_num = random.choice(list(self.active_orders.keys()))
                    f.write(self.delete_order(ref_num))

            # End of Day
            f.write(self.system_event('C'))

if __name__ == "__main__":
    import os
    data_dir = "data"
    if not os.path.exists(data_dir):
        os.makedirs(data_dir)
    
    output_file = os.path.join(data_dir, "synthetic.itch")
    gen = ITCHGenerator(output_file)
    gen.generate(500000)
    print(f"Generated 500000 ITCH messages to {output_file}")
