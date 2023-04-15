#pragma once

void InitializeNetwork();
void GetPointCloud(int* size, int* size_2, float** points, float** points_2);

extern int player_delay;
extern int monster_delay;
extern std::atomic_int active_clients;
extern std::atomic_int active_monsters;