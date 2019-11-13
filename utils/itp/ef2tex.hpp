#ifndef EF2TEX_HPP
#define EF2TEX_HPP

class Tex_Man_t;
class Lem_Gdb_t;

Tex_Man_t * Lem_StartTexFromEf( Lem_Gdb_t * );
void Lem_StartNetFromEf( Lem_Gdb_t *, Tex_Man_t * );
void Lem_StartObsFromEf( Lem_Gdb_t *, Tex_Man_t * );
#endif