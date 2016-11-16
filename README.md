The motivation for this library is to explore the separation between the algorithm for managing a game and the user interface for the game. Discovering a TicTacToe algorithm simple enough to implement on the Arduino allows an exploration of this concept with simple user interface requirements.

The integration between game library and user interface is largely implemented through a callback function that allows the user program to update each game cell as the game progresses, without needing detailed knowledge of the game board and current positions. The example programs supplied provide a framework that can be changed for alternative user interfaces.

The code implements an elegant algorithm that is briefly described [here](http://www.omnimaga.org/math-and-science/tic-tac-toe-algorithm/) with some additional features to make decisions easier to implement in code. 

Please read the html documentation in the _doc_ folder of the library.

[Library Documentation](https://majicdesigns.github.io/TicTacToe/)