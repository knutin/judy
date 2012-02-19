-module(judy).
-compile(export_all).

-on_load(init/0).

init() ->
    EbinDir = filename:dirname(code:which(?MODULE)),
    AppPath = filename:dirname(EbinDir),
    Path = filename:join([AppPath, "priv", "judy"]),
    ok = erlang:load_nif(Path, 0).

new(_MaxKeys, _ValueSize) ->
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
    {ok, J} = new(10000, 2),
    ?assertEqual(ok, insert(J, <<"foobar">>, <<1, 1>>)),
    ?assertEqual(<<1, 1>>, get(J, <<"foobar">>)),

    ?assertEqual(ok, insert(J, <<"bazoka">>, <<100, 100>>)),
    ?assertEqual(<<100, 100>>, get(J, <<"bazoka">>)),

    ?assertEqual(ok, update(J, <<"foobar">>, <<2, 2>>)),
    ?assertEqual(<<2, 2>>, get(J, <<"foobar">>)),

    ?assertEqual(ok, update(J, <<"bazoka">>, <<3, 3>>)),
    ?assertEqual(<<3, 3>>, get(J, <<"bazoka">>)),

    ?assertError(badarg, insert(J, <<"foobar">>, <<2, 2>>)),
    ?assertError(badarg, insert(J, <<"baz">>, <<2, 2, 3>>)),

    ?assertEqual(ok, delete(J, <<"foobar">>)),
    ?assertEqual(not_found, get(J, <<"foobar">>)).


num_keys_test() ->
    {ok, J} = new(100, 1),
    ?assertEqual(0, num_keys(J)),
    ?assertEqual(ok, insert(J, <<1>>, <<1>>)),
    ?assertEqual(ok, insert(J, <<2>>, <<2>>)),
    ?assertEqual(2, num_keys(J)).

max_keys_test() ->
    {ok, J} = new(3, 1),
    ?assertEqual(ok, insert(J, <<1>>, <<1>>)),
    ?assertEqual(ok, insert(J, <<2>>, <<1>>)),
    ?assertEqual(ok, insert(J, <<3>>, <<1>>)),
    ?assertError(badarg, insert(J, <<4>>, <<1>>)),
    ?assertEqual(3, num_keys(J)).


leak_test() ->
    N = 10000000,
    {ok, J} = new(N, 2),
    leak_test(J, 1, N).
    %% leak_test(J, 1, N),
    %% leak_test(J, 1, N),
    %% leak_test(J, 1, N),
    %% leak_test(J, 1, N).

leak_test(_, N, N) ->
    ok;
leak_test(J, Start, N) ->
    case get(J, <<Start:64/integer>>) of
        not_found ->
            ok = insert(J, <<Start:64/integer>>, <<Start:16/integer>>);
        _Val ->
            ok = update(J, <<Start:64/integer>>, <<Start:16/integer>>)
    end,
    leak_test(J, Start+1, N).



perf_test_() ->
    {timeout, 600,
     fun() ->
             N = 1000000,
             {ok, J} = new(N, 2),
             error_logger:info_msg("VM before populating: ~p mb~n",
                                   [trunc(erlang:memory(total) / 1024 / 1024)]),
             populate(J, 1, N),
             error_logger:info_msg("keys: ~p~n", [num_keys(J)]),
             error_logger:info_msg("VM with keys: ~p mb~n",
                                   [trunc(erlang:memory(total) / 1024 / 1024)]),

             time_reads(J, 1, N),

             receive after infinity -> ok end
     end}.


populate(_, N, N) ->
    ok;
populate(J, Start, N) ->
    ok = insert(J, <<Start:64/integer>>, <<Start:16/integer>>),
    populate(J, Start+1, N).


time_reads(J, Start, N) ->
    Reps = 10,
    StartTime = now(),
    [check_values(J, Start, N) || _ <- lists:seq(1, Reps)],
    ElapsedUs = timer:now_diff(now(), StartTime),

    error_logger:info_msg(
      "REPORT~n"
      "Keys: ~p~n"
      "Average of 20 runs: ~p us~n"
      "Estimated per read: ~p us~n"
      "Theoretical sequential RPS: ~p~n",
      [N, ElapsedUs / Reps, ElapsedUs / N / Reps,
       trunc(1000000 / (ElapsedUs / N / Reps))]),

    ok.


check_values(_, N, N) ->
    ok;
check_values(J, Start, N) ->
    ?assertEqual(<<Start:16/integer>>, get(J, <<Start:64/integer>>)),
    check_values(J, Start+1, N).
