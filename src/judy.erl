-module(judy).
-compile(export_all).

-on_load(init/0).

init() ->
    EbinDir = filename:dirname(code:which(?MODULE)),
    AppPath = filename:dirname(EbinDir),
    Path = filename:join([AppPath, "priv", "judy"]),
    ok = erlang:load_nif(Path, 0).

new() ->
    exit(nif_not_loaded).

insert(_J, _Key, _Value) ->
    exit(nif_not_loaded).

update(_J, _Key, _Value) ->
    exit(nif_not_loaded).

delete(_J, _Key) ->
    exit(nif_not_loaded).

get(_J, _Key) ->
    exit(nif_not_loaded).


num_keys(_J) ->
    exit(nif_not_loaded).

-include_lib("eunit/include/eunit.hrl").


operations_test() ->
    {ok, J} = new(),
    ?assertEqual(ok, insert(J, <<"foobar">>, <<"value">>)),
    ?assertEqual(<<"value">>, get(J, <<"foobar">>)),
    ?assertEqual(ok, update(J, <<"foobar">>, <<"new">>)),
    ?assertEqual(<<"new">>, get(J, <<"foobar">>)),
    ?assertError(badarg, insert(J, <<"foobar">>, <<"other">>)),

    ?assertEqual(ok, delete(J, <<"foobar">>)),
    ?assertEqual(not_found, get(J, <<"foobar">>)).


num_keys_test() ->
    {ok, J} = new(),
    ?assertEqual(0, num_keys(J)),
    ?assertEqual(ok, insert(J, <<1>>, <<1>>)),
    ?assertEqual(ok, insert(J, <<2>>, <<2>>)),
    ?assertEqual(2, num_keys(J)).


size_test_() ->
    {timeout, 600,
     fun() ->
             {ok, J} = new(),
             populate(J, 1, 1000000),
             error_logger:info_msg("keys: ~p~n", [num_keys(J)]),
             error_logger:info_msg("VM: ~p mb~n", [erlang:memory(total) / 1024 / 1024]),
             receive after infinity -> ok end
     end}.


populate(_, N, N) ->
    ok;
populate(J, Start, N) ->
    ok = insert(J, <<Start:64/integer>>, <<100:16/integer>>),
    populate(J, Start+1, N).
