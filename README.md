# Reinforcement Learning for Dummies

A deliberately minimal, from-scratch demonstration of **how reinforcement learning
works** — using the classic 1960s "matchbox learning machine" idea applied to the
game of **TRIS** (tic-tac-toe).

## Why this project exists

The goal is **not** to build a strong TRIS engine. The goal is to show the
**principles** of reinforcement learning in the simplest possible way, and then,
across successive versions, to show how the same system can be made to **learn
better and play better**.

To keep things understandable, the early versions **do not use neural networks at
all**. Instead, every cell carries a weight. 
Cells with higher weights are chosen more often. 
Learning happens by increasing weights under specific conditions.

That's the whole idea. 
No gradients, no tensors — just integers on a board that get
rewarded when they belong to a good game.

## How the system learns (the reward rule)

A game ends in one of two ways: * **Win **, **Pair**.

When the game finishes, the moves that were played are updated, starting from the
**last move** and walking back to the first:

* **On a win**, the winner's moves get **+3** and the loser's moves get **-3**
  (a weight never drops below a small floor, so no move ever becomes impossible).
* **On a pair**, all played moves get **+1**.
* Every step back in time the update is multiplied by a **discount factor (0.8)**:
  the move that ended the game gets full credit (or blame), the opening move only a
  fraction. The blunder that lost a game is punished hard; the innocent early moves
  are barely touched.

Good moves gradually accumulate more "pull" and the system starts favouring the
paths that historically worked.

## Workflow

The program is a single binary with three modes.
Is possible to launch the program using two modes:
- `run.sh`: compile and run
- `./tris`: after the compile phase, just run

### 1. Learn — `./run.sh -r <epics> [<games>]`

Runs the automatic self-play learning loop. 
Runs `<epics>` cycles. Each composed by `<games>` matches.

If `games` is omitted. The defult value is `100.000`.
Every cycle (`epic`) generate a file `game-<epic_id>.txt` that dumps the status of the game.


### 2. Normalize / consolidate — `./run.sh -l`

Normalize the result of the execution of the `<epics>`  into a single, normalized file.
By convention with the name `game-100.txt`.
The normalization is done by keeping the board with the higher sum of weights.

### 3. Play — `./run.sh -i`


Loads the consolidated weights (`game-100.txt`) and lets **you** play against the
learned agent interactively.
Sides are assigned at random.

##  full example

```bash
./run.sh -r 10 100000 # 10 epic of 100.000 games each
./run.sh -l           # normalize
./run.sh -i           # play
```

## Release notes

### 0.0.1 — basic pre-fill (weighted-roulette) algorithm

**Observed behaviour:** the agent already makes *reasonable* choices but plays
**poorly** overall. Even with large training sets the number of explored board
combinations barely reaches ~**8.8k**, so coverage of the state space is thin and the
learned policy is shallow. Later versions will address this — first by improving the
learning/exploration rule, and eventually by evolving the representation itself.

### 0.0.2 — reward fixes and discounting

* fixed a bug where the **winning move itself was never rewarded** (the strongest
  learning signal was dropped every game).
* reward scale raised from ~0.02/game to **+3 / +1** as documented, so weights
  actually move relative to the default weight of 2.
* updates are now **discounted (0.8 per step)** walking back from the final move,
  so credit and blame concentrate on the moves closest to the outcome.

**Observed behaviour:** after a single 20k-game epic the agent already strongly
favours the centre reply to a corner/edge opening (weight ~50–115 vs ~0.1–1 for
the alternatives) — something the old reward scale never achieved.
