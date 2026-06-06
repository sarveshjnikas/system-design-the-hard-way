# System Designs

Implementations of classic system design problems.
1. Design in detail on whiteboard.
2. Implement as much as I can in as much detail as I can.
3. Suffer and sweat while doing so (basically no vibe coding!)

---

## DESIGN 0: Rate Limiter

![Rate Limiter Design](0-rate-limiter/docs/design.png)

Token bucket rate limiter built in Go with Redis.

- Per-account token buckets stored in Redis
- Atomic Lua script — no race conditions
- Configurable via environment variables (`MAX_TOKENS`, `INTERVAL`)
- Returns `429 Too Many Requests` when limit is exceeded

[Notes](rate-limiter/docs/design.txt) · [Code](rate-limiter/)

## DESIGN 1: Consistent Hashing

- Virtual nodes for uniform key distribution
- O(log n) key lookup via sorted ring (`std::map` + `lower_bound`)
- O(virtual_node_count) server removal via reverse index
- MurmurHash3 — same hash function used by Cassandra