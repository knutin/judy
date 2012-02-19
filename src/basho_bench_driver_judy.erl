-module(basho_bench_driver_judy).

-export([new/1,
         run/4]).

new(_Id) ->
    InitialKeys = basho_bench_config:get(initial_keys),
    judy:new(InitialKeys, 2).



run(mget, KeyGen, _ValueGen, P) ->
    NumKeys = basho_bench_config:get(mget_keys),
    StartKey = KeyGen(),
    Keys = [<<I:64/integer>> || I <- lists:seq(StartKey, StartKey + (NumKeys * 1000), 1000)],

    case catch(judy:mget(P, Keys)) of
        Value when is_list(Value) ->
            {ok, P};
        {error, Reason} ->
            {error, Reason, P};
        {'EXIT', {timeout, _}} ->
            {error, timeout, P}
    end.
