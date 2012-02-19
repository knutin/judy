### Judy - playground for learning C and NIFs

This project provides a NIF wrapper around Judy arrays and a chunk of
memory, allowing memory efficient storage and fast retrieval of
values. The library allocates and manages a large enough chunk of
memory to store the values, using the Judy array for finding the
proper offset into this chunk. Deletion is currently not supported.


Usage:
```erlang

    {ok, J} = judy:new(100, 2).
    ok = judy:insert(J, <<"some key">>, <<123, 123>>).
    <<123, 123>> = judy:get(J, <<"some key">>).

```

See eunit tests in `src/judy.erl` for more examples.
