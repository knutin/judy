#include <stdio.h>
#include <string.h>
#include <Judy.h>
#include "erl_nif.h"
#include "judy.h"


char* mk_value(unsigned char* value, size_t size)
{
  char* v = enif_alloc(size);
  memcpy(v, value, size);

  return v;
}

void free_value(char* v)
{
  enif_free(v);
}


static ERL_NIF_TERM judy_new(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  unsigned long max_keys;
  int value_size;

  if (enif_get_ulong(env, argv[0], &max_keys) &&
      enif_get_int(env, argv[1], &value_size))
    {
      judy_t* judy = (judy_t*)enif_alloc_resource(JUDY_RESOURCE, sizeof(judy_t));

      judy->judy = NULL;
      judy->max_keys = max_keys;
      judy->value_size = (size_t)value_size;

      judy->buf = enif_alloc(judy->value_size * judy->max_keys);
      judy->last_index = 0;
      judy->num_keys = 0;


      ERL_NIF_TERM result = enif_make_resource(env, judy);
      enif_release_resource(judy);

      return enif_make_tuple2(env, enif_make_atom(env, "ok"), result);
    } else {
    return enif_make_badarg(env);
  }
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

    if((size_t)value.size != judy->value_size) {
      return enif_make_badarg(env);
    }

    JHSG(PValue, judy->judy, key.data, key.size);

    if(PValue != NULL) {
      return enif_make_badarg(env);
    }

    unsigned long i = judy->last_index++;
    if(i == judy->max_keys) {
      return enif_make_badarg(env);
    }

    char* v = &judy->buf[i*judy->value_size];
    memcpy(v, value.data, judy->value_size);

    JHSI(PValue, judy->judy, key.data, key.size);
    *PValue = (Word_t)i;

    judy->num_keys++;

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

    unsigned long i = (int)*PValue;

    ERL_NIF_TERM result_term;
    unsigned char* result_buf = enif_make_new_binary(env, judy->value_size, &result_term);
    memcpy(result_buf, &judy->buf[i*judy->value_size], judy->value_size);

    return result_term;

  } else {
    return enif_make_badarg(env);
  }
}

ERL_NIF_TERM judy_update(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  ErlNifBinary key;
  ErlNifBinary new_value;
  judy_t* judy;
  PWord_t PValue;

  if (enif_get_resource(env, argv[0], JUDY_RESOURCE, (void**)&judy) &&
      enif_inspect_binary(env, argv[1], &key) &&
      enif_inspect_binary(env, argv[2], &new_value)) {

    JHSG(PValue, judy->judy, key.data, key.size);

    if(PValue == NULL) {
      return enif_make_badarg(env);
    }

    unsigned long i = (unsigned long)*PValue;

    char* v = &judy->buf[i*judy->value_size];
    memcpy(v, new_value.data, judy->value_size);

    JHSI(PValue, judy->judy, key.data, key.size);
    *PValue = (Word_t)i;

    return enif_make_atom(env, "ok");
  } else {
    return enif_make_badarg(env);
  }
}

ERL_NIF_TERM judy_delete(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  ErlNifBinary key;
  judy_t* judy;
  int delete_result;
  PWord_t* PValue;

  if (enif_get_resource(env, argv[0], JUDY_RESOURCE, (void**)&judy) &&
      enif_inspect_binary(env, argv[1], &key)) {

    JHSG(PValue, judy->judy, key.data, key.size);

    if(PValue == NULL) {
      return enif_make_badarg(env);
    }

    unsigned long i = (unsigned long)*PValue;
    memset(&judy->buf[i], '\0', judy->value_size);

    JHSD(delete_result, judy->judy, key.data, key.size);

    judy->num_keys--;

    return enif_make_atom(env, "ok");
  } else {
    return enif_make_badarg(env);
  }
}

ERL_NIF_TERM judy_num_keys(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  judy_t* judy;

  if (enif_get_resource(env, argv[0], JUDY_RESOURCE, (void**)&judy)) {
    return enif_make_int(env, judy->num_keys);
  } else {
    return enif_make_badarg(env);
  }
}


void judy_dtor(ErlNifEnv* env, void* arg)
{
  judy_t* judy = (judy_t*)arg;
  Word_t bytes;
  JHSFA(bytes, judy->judy);
  fprintf(stderr, "freed %d bytes\n", bytes);
}

int on_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
  ErlNifResourceFlags flags = (ErlNifResourceFlags)(ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER);
  JUDY_RESOURCE = enif_open_resource_type(env, NULL, "judy_resource",
                                          &judy_dtor, flags, 0);
  return 0;
}

static int
reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info)
{
    return 0;
}

static int
upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info)
{
    *priv = *old_priv;
    return 0;
}

static void
unload(ErlNifEnv* env, void* priv)
{
    enif_free(priv);
    return;
}

static ErlNifFunc nif_funcs[] = {
  {"new", 2, judy_new},
  {"insert", 3, judy_insert},
  {"get", 2, judy_get},
  {"update", 3, judy_update},
  {"delete", 2, judy_delete},
  {"num_keys", 1, judy_num_keys}
};

ERL_NIF_INIT(judy, nif_funcs, &on_load, &reload, &upgrade, &unload)
