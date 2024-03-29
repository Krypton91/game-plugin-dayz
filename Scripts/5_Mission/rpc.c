

class GameLabsRPC {
    void GameLabsRPC() {
        GetDayZGame().Event_OnRPC.Insert( OnRPC );
    }

    void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx) {
        if(GetGame().IsServer()) HandleServerRPC(sender, target, rpc_type, ctx);
        else HandleClientRPC(sender, target, rpc_type, ctx);
    }

    /*
     * TODO: PoC. This is inefficient beyond reasonable doubt, implement proper sync
     */

    private void HandleClientRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx) {
        switch(rpc_type) {
            case GameLabsRPCS.RE_SERVERFPS: {
                ref Param1<float> response = new Param1<float>(-1.0);
                ctx.Read(response);

                float serverFps = response.param1;
                GetGameLabs().SetServerFPS(serverFps);
                break;
            }
        }
    }

    PlayerBase GetPlayerByIdentity(PlayerIdentity identity) {
        ref array<Man> players = new array<Man>;
        GetGame().GetPlayers( players );

        for( int i = 0; i < players.Count(); i++) {
            PlayerBase player;
            Class.CastTo(player, players.Get(i));
            if(player) {
                if(player.GetPlainId() == identity.GetPlainId()) return player;
            }
        }
        return NULL;
    }

    private void HandleServerRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx) {
        switch(rpc_type) {
            case GameLabsRPCS.RQ_SERVERFPS: {
                ref Param1<float> payloadRQServerFps = new Param1<float>(GetGameLabs().GetServerFPS());
                GetGame().RPCSingleParam(null, GameLabsRPCS.RE_SERVERFPS, payloadRQServerFps, true, sender);
                return;
            }
            case GameLabsRPCS.SY_EXPANSIONCHAT: {
                ChatMessageEventParams data;
                if(!ctx.Read(data)) return;
                if(!sender) return;

                PlayerBase player = GetPlayerByIdentity(sender);
                if(!player) return;

                string channel;
                if(data.param1 == 128) {
                    channel = "side";
                } else if(data.param1 == 256){
                    channel = "team";
                } else {
                    channel = "external";
                }

                ref _LogPlayerEx logObjectPlayer = new _LogPlayerEx(player);
                ref _Payload_PlayerChat payloadExpansionChat = new _Payload_PlayerChat(logObjectPlayer, channel, data.param3);
                GetGameLabs().GetApi().PlayerChat(new _Callback(), payloadExpansionChat);
            }
        }
    }
};
