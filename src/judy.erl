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

get(_J, _Key) ->
    exit(nif_not_loaded).


-include_lib("eunit/include/eunit.hrl").


simple_test() ->
    {ok, J} = new(),
    error_logger:info_msg("J: ~p~n", [J]),
    ?assertEqual(ok, insert(J, <<"foobar">>, <<"value">>)),
    ?assertEqual(<<"value">>, get(J, <<"foobar">>)),
    ok.
