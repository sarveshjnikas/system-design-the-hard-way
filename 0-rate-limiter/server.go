package main
import (
	"context"
	"fmt"
	"log"
	"net/http"
	"github.com/redis/go-redis/v9"
	"strconv"
	"time"
	"rate-limiter/config"
)

var ctx =  context.Background()
var rdb = redis.NewClient(&redis.Options{
	Addr: "localhost:6379",
	Password: "",
	DB: 0,
})
var cfg = config.Load()

var RateLimitScript = redis.NewScript(`
local tokens_key = KEYS[1]
local last_key = KEYS[2]

local max_tokens = tonumber(ARGV[1])
local interval = tonumber(ARGV[2])
local now = tonumber(ARGV[3])

local tokens = tonumber(redis.call('GET', tokens_key))
local last = tonumber(redis.call('GET', last_key))

if not tokens and not last then
	redis.call('SET', tokens_key, max_tokens-1)
	redis.call('SET',last_key , now)
	return 1
end

local elapsed = now - last
local tokens_to_add = math.floor(elapsed / interval) * max_tokens
local new_token_count = math.min(tokens + tokens_to_add, max_tokens)

if new_token_count <= 0 then
    return 0
end

redis.call('SET', tokens_key, new_token_count-1)
redis.call('SET',last_key , now)

return 1`)

func handler(w http.ResponseWriter, r *http.Request){
	accountID, err := strconv.Atoi(r.URL.Path[1:])
	if err != nil {
		http.Error(w, "invalid accountid", http.StatusNotFound)
		return
	}

	is_allowed := allow(rdb, accountID)
	if !is_allowed {
		http.Error(w, "too many requests try again later", http.StatusTooManyRequests)
		return
	}
	fmt.Fprintf(w, "%s" , r.URL.Path[1:])
}

func allow(rdb *redis.Client, accountID int) bool {
	INTERVAL := cfg.Interval
	MAX_TOKENS := cfg.MaxTokens
	now := time.Now().Unix()

	result, err := RateLimitScript.Run(ctx, rdb, []string {strconv.Itoa(accountID) + ":tokens", strconv.Itoa(accountID) + ":last"}, MAX_TOKENS, INTERVAL, now ).Result()
	if err != nil {
		log.Println(err)
		return false
	}
	return result.(int64) == 1
}

func main(){
	
	http.HandleFunc("/", handler)
	log.Fatal(http.ListenAndServe(":8080", nil))
}