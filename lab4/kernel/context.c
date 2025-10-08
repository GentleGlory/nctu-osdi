#include "context.h"


void context_switch(struct task *next)
{
	struct task *prev = current;
	switch_to(prev, next);
}