package config

import (
	"os"
	"strconv"
)

type Config struct {
	Interval int
	MaxTokens int
}

func Load() Config {
	maxTokensStr := os.Getenv("MAX_TOKENS")
	if maxTokensStr == "" {
		maxTokensStr = "5"
	}
	maxTokens, _ := strconv.Atoi(maxTokensStr)
	
	intervalStr := os.Getenv("INTERVAL")
	if intervalStr == "" {
		intervalStr = "60"
	}
	interval, _ := strconv.Atoi(intervalStr)


	return Config{
		Interval:  interval,
		MaxTokens: maxTokens,
	}
}