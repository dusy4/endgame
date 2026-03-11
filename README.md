# The Grand Wager

A casino-style game built with C and Raylib.

Watch DEMO:
[![Watch the demo](https://img.youtube.com/vi/zDrPrxbzvnA/maxresdefault.jpg)](https://www.youtube.com/watch?v=zDrPrxbzvnA)

## Goal
Increase your global balance (starting at $1000) by playing various mini-games. The ultimate goal is to reach $1,000,000 to unlock the Ending.

## Idea & Concept
The game simulates a virtual gambling environment offering multiple classic casino mini-games. Players risk their virtual money in games of chance, balancing risk and reward to maximize their payouts.

## Logic / Features
The game operates on a state machine switching between different menus and mini-games:
- **Rocket Game (Crash):** Place a bet and cash out before the rocket crashes. The longer you wait, the higher the multiplier.
- **Star Game (Minesweeper):** Click tiles to reveal stars (increasing multiplier) and avoid bombs. Cash out your winnings before hitting a bomb.
- **Hippodrome Game (Horse Racing):** Bet on 1 of 10 horses. Speeds and win probabilities are dynamically calculated during the race. 
- **Slots Game:** Classic 3-reel slot machine with varying payout combinations, including jackpots, free spins, and double wins.

## Build and Run
```bash
make
./game
```
