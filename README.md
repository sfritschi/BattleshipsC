# BattleshipsC
Command Line Battleships Game to be played over the local network.

Player HOSTING the game runs:
./battle h

Player JOINING the game runs:
./battle j
And also enters the hostname displayed for the host

## Rules
- Each player places the 5 ships within their player board.
- Once both players have finished placing their ships, the boards are exchanged over the network.
- The player that joined the game goes first and can enter the shooting coordinates.
- Players each take turns at firing shots until all ship parts of one of the two players are destroyed.
- At the end both players are asked if they would like to play again.
