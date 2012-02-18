#include <stdio.h>
#include <string.h>
#include <Judy.h>
#include "erl_nif.h"
#include "judy.h"


judy_value* mk_value(unsigned char* value, size_t size)
{
  judy_value* jv = malloc(sizeof(judy_value));
  jv->size = size;
  jv->value = malloc(size);
  memcpy(jv->value, value, size);

  return jv;
}


static ERL_NIF_TERM judy_new(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  judy_t* judy = (judy_t*)enif_alloc_resource(JUDY_RESOURCE, sizeof(judy_t));
  judy->judy = NULL;

  ERL_NIF_TERM result = enif_make_resource(env, judy);
  enif_release_resource(judy);
  return enif_make_tuple2(env, enif_make_atom(env, "ok"), result);
}

ERL_NIF_TERM judy_insert(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  ErlNifBinary key;
  ErlNifBinary value;
  judy_t* judy;

  PWord_t PValue;

  if (enif_get_resource(env, argv[0], JUDY_RESOURCE, (void**)&judy) &&
      enif_inspect_binary(env, argv[1], &key) &&
      enif_inspect_binary(env, argv[2], &value)) {

    judy_value* v = mk_value(value.data, value.size);

    JHSI(PValue, judy->judy, key.data, key.size);
    *PValue = (Word_t)v;

    return enif_make_atom(env, "ok");
  } else {
    return enif_make_badarg(env);
  }
}



ERL_NIF_TERM judy_get(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  ErlNifBinary key;
  judy_t* judy;
  PWord_t PValue;

  if (enif_get_resource(env, argv[0], JUDY_RESOURCE, (void**)&judy) &&
      enif_inspect_binary(env, argv[1], &key)) {

    JHSG(PValue, judy->judy, key.data, key.size);

    if(PValue == NULL) {
      return enif_make_atom(env, "not_found");
    }

    judy_value* v = (judy_value*)*PValue;

    ERL_NIF_TERM result_term;
    unsigned char* result_buf = enif_make_new_binary(env, v->size, &result_term);
    memcpy(result_buf, v->value, (size_t)v->size);

    return result_term;

  } else {
    return enif_make_badarg(env);
  }
}


void judy_dtor(ErlNifEnv* env, void* arg)
{
  /* Word_t bytes; */
  /* dprintf("judy %p\n", judy); */
  /* JHSFA(bytes, judy); */
}

int on_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
  ErlNifResourceFlags flags = (ErlNifResourceFlags)(ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER);
  JUDY_RESOURCE = enif_open_resource_type(env, NULL, "judy_resource",
                                          &judy_dtor,
                                          flags,
                                          0);
  return 0;
}


static ErlNifFunc nif_funcs[] = {
  {"new", 0, judy_new},
  {"insert", 3, judy_insert},
  {"get", 2, judy_get}
};

ERL_NIF_INIT(judy, nif_funcs, &on_load, NULL, NULL, NULL)
