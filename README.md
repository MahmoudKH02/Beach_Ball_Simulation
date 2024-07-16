
# Table of Contents
- [Table of Contents](#table-of-contents)
- [Main Idea](#main-idea)
- [Simulation Rules](#simulation-rules)
- [Simulation Demo](#simulation-demo)
- [Implementation ](#implementation-)
  - [Files:](#files)
  - [Used IPC Techniques:](#used-ipc-techniques)
    - [Signals (Interrupts):](#signals-interrupts)
    - [Unnamed Pipes](#unnamed-pipes)
    - [Named Pipes (FIFOs)](#named-pipes-fifos)
- [How to Run](#how-to-run)
- [Collaborators](#collaborators)

<br>

# Main Idea
This project simulates the beach ball game, in C language (with OpenGL graphics), using a Multiprocessing approach.

# Simulation Rules
There are two teams in the simulation, Team A, and Team B, and a parent process. Each Team consists of 5 players and a team lead (6 players each team).

1. The Simulation starts with a ball with each of the team leads.
2. Then each team lead passes the ball to the next player in team (team lead -> player 1).
3. Each player passes the ball to the next one (player 1 -> player 2, 2 -> 3), and so on until the ball reaches player 6 in the team.
4. After the ball is passed to player 6, then that player passes the ball back to his team lead.
5. The team lead (Team A) passes the ball to the team lead of the other team (Team B), and now Team B has two balls.
6. When there is no balls with a team, the team lead requests a ball from the parent process, (i.e. Team A has no balls, so it requests a ball from parent), and now, there are 3 balls in the game.

***Note:*** When a player receives a ball, there is a chance it would be dropped, depending on the energy of that player, *when the Text above a player turns red, this means it has dropped the ball.*

# Simulation Demo
https://github.com/MahmoudKH02/Beach_Ball_Simulation/assets/164411438/2724feaa-f4be-49aa-899c-aa9e0c43826a

# Implementation <a name="mm"></a>

## Files:
* `player.c`: Normal Player process.
* `teamLead.c`: The team lead player process.
* `drawer.c`: Process responsible for OpenGl graphics.
* `main.c`: The Parent process.


## Used IPC Techniques:
* Signals (Interrupts).
* Unnamed pipes.
* Named Pipes (FIFOs).

### Signals (Interrupts):
The ball is represented by a signal in this simulation, so when a player passes a ball to the next player, it sends a signal to that player using the `kill()` function.

After the player passes the ball it has, then it would `pause()` and will wake up only when a signal is received.

### Unnamed Pipes
The unnamed pipes are used to pass the necessary pid from parent process to the players, because when a signal is sent from one process to another, the pid of the receiving process is required by the process that sends the signal using the `kill()` function, more on that function [HERE](https://www.ibm.com/docs/it/zos/2.4.0?topic=functions-kill-send-signal-process) , or use the following command in the terminal to view the manual.
```
man 2 kill
```

### Named Pipes (FIFOs)
The FIFOs are used to pass information from each player (including team leads), to the OpenGl process, since the OpenGl is a separate process.

<br>

# How to Run
Just enter the command `make`, in the terminal, and it will compile all the needed files, then run `./main`
```
make
./main
```
this would use the default settings (which are found in the settings.txt), you can use your own settings file by passing an argument to the main, like this:

*Replace <your_settings>.txt with the file name you have.*

```
make
./main <your_settings>.txt
```
<br>

# Collaborators
* [MahmoudKH02](https://github.com/MahmoudKH02)
* [AhmadBakri7](https://github.com/AhmadBakri7)
* [ahmadghz17](https://github.com/ahmadghz17)
* [Ayman-Salama](https://github.com/Ayman-Salama)

