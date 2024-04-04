#ifndef __FUNCTIONS__
#define __FUNCTIONS__

unsigned int get_sleep_duration(int energy, int balls_with_player, int player_num);
pid_t fetch_next_pid(int r_fd_pipe, int* other_team_lead);
void init_vars(int* energy, int* num_balls_player, int* num_balls_team);

#endif