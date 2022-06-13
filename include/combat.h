#ifndef COMBAT_H
#define COMBAT_H

int do_attack(struct actor *, struct actor *);
int weak_res(unsigned short, unsigned short, unsigned short);
int calculate_evasion(struct actor *);
int calculate_accuracy(struct actor *, struct attack *);
int cycle_active_attack(void);
struct attack *get_active_attack(void);

#endif