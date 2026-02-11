#ifndef DEBUG_API_H
#define DEBUG_API_H

#if HANDMADE_FAST
#define Assert(Expression)
#else
#define Assert(Expression) if(!(Expression)) {*(int * )0 = 0;}
#endif


#endif // DEBUG_API_H