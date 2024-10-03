This is a chess engine currently in development. Created by Nathanael Mohanu. <br />

Note: The move generator is already quite optimized and late stage. The AI and GUI are working but in early development. Currently only working on ARM cpu's.

Project contains an SFML GUI.<br />
<br />
Features: <br />
<br />
Move generator: <br />
- Bitboards.<br />
- Binary piece representation.<br />
- Magic bitboards / lookuptables.<br />
- PERFT test (perfect score tested up to depth 8).<br />
- Zobrish hashing <br />
<br />
GUI:<br />
- GUI allowing the player to play a game.<br />
- Visuals for reach, piece color & more.<br />
- GUI works on a seperate thread, GUI keeps running while AI is searching for moves. <br />
<br />
AI:<br />
- Minimax algorithm. <br />
- Alpha beta pruning. <br />
- Zobrist hashing. <br />
- Iterative deepening.
<br />
<br />
Currently working on:<br />
- Move ordering <br />
- More pruning techniques. <br />
