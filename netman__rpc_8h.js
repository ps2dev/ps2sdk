var netman__rpc_8h =
[
    [ "NetManEEInitResult", "netman__rpc_8h.html#struct_net_man_e_e_init_result", [
      [ "result", "netman__rpc_8h.html#aac2ff2a04b29d21817ebdb0574d350aa", null ],
      [ "FrameBufferStatus", "netman__rpc_8h.html#a988ce48a971c81a3f4f1f44b9b6b77e1", null ]
    ] ],
    [ "NetManRegNetworkStack", "netman__rpc_8h.html#struct_net_man_reg_network_stack", [
      [ "FrameBufferStatus", "netman__rpc_8h.html#a8cbcb2ab9d2839f6c7e2353894db0bda", null ]
    ] ],
    [ "NetManRegNetworkStackResult", "netman__rpc_8h.html#struct_net_man_reg_network_stack_result", [
      [ "result", "netman__rpc_8h.html#ad82f5851676b972138585328b421c3ea", null ],
      [ "FrameBuffer", "netman__rpc_8h.html#a560f9a1ec348e5e124f5906505027bd0", null ],
      [ "FrameBufferStatus", "netman__rpc_8h.html#a484a7c271669e08c49274dccf1e9d62c", null ]
    ] ],
    [ "NetManQueryMainNetIFResult", "netman__rpc_8h.html#struct_net_man_query_main_net_i_f_result", [
      [ "result", "netman__rpc_8h.html#afe526ea6d6e53a97c05767d11bdcf62c", null ],
      [ "name", "netman__rpc_8h.html#a239e7365985274e9ca4d9e9ee6c3bd1a", null ]
    ] ],
    [ "NetManIoctl", "netman__rpc_8h.html#struct_net_man_ioctl", [
      [ "command", "netman__rpc_8h.html#a0398f7577d6ecb272ada896fb47c4474", null ],
      [ "args", "netman__rpc_8h.html#a6be518ec08afe254f2d889d8bb95fc87", null ],
      [ "args_len", "netman__rpc_8h.html#a6771f32f6ea591fe0e00e69bfe315a25", null ],
      [ "output", "netman__rpc_8h.html#a9f4383774165e700300bc2ad41247432", null ],
      [ "length", "netman__rpc_8h.html#acaa6cfb74b8bf2193e614919c08a5445", null ]
    ] ],
    [ "NetManIoctlResult", "netman__rpc_8h.html#struct_net_man_ioctl_result", [
      [ "result", "netman__rpc_8h.html#a531e65d7ce8a80fe098a5c568196aa53", null ],
      [ "output", "netman__rpc_8h.html#a07dd1dced9c4f704fe618499f303eb4a", null ]
    ] ],
    [ "NetManPktCmd", "netman__rpc_8h.html#struct_net_man_pkt_cmd", [
      [ "id", "netman__rpc_8h.html#a15e789699cac6034d8e2c39345db9498", null ],
      [ "offset", "netman__rpc_8h.html#a5e43cb6795739d458c26c84f23d680ff", null ],
      [ "length", "netman__rpc_8h.html#ab4cb1f3300e68edbe6fa6fcb74c66479", null ]
    ] ],
    [ "NetManBD", "netman__rpc_8h.html#struct_net_man_b_d", [
      [ "length", "netman__rpc_8h.html#a68015952b021658c26be717ec1e88d8b", null ],
      [ "offset", "netman__rpc_8h.html#a08db909d3e138daa5883e9f9d48d3a5d", null ],
      [ "packet", "netman__rpc_8h.html#a70305c07e276e6520475bb41968dcf55", null ],
      [ "payload", "netman__rpc_8h.html#adfffaf42357503f31c0376c9d94392c0", null ],
      [ "unused", "netman__rpc_8h.html#ae00836683f925c3ffcf7849e6b3416ee", null ]
    ] ],
    [ "NETMAN_RPC_NUMBER", "netman__rpc_8h.html#a8d186b72313b69fcf8cb65975e8462c6", null ],
    [ "NETMAN_SIFCMD_ID", "netman__rpc_8h.html#a9f199630d2da97c3ab2961a2b152505a", null ],
    [ "NETMAN_MAX_FRAME_SIZE", "netman__rpc_8h.html#a504a3bc8cb8c6c810508f5a73ab5d8aa", null ],
    [ "NETMAN_RPC_BLOCK_SIZE", "netman__rpc_8h.html#a452f90d892d86683d439989c027e149e", null ],
    [ "NETMAN_EE_RPC_FUNC_NUMS", "netman__rpc_8h.html#a2e940d1ee6e0c6c37ab8d9f3549cd419", [
      [ "NETMAN_EE_RPC_FUNC_INIT", "netman__rpc_8h.html#a2e940d1ee6e0c6c37ab8d9f3549cd419ad292ae989392db1314353f1750f7a457", null ],
      [ "NETMAN_EE_RPC_FUNC_DEINIT", "netman__rpc_8h.html#a2e940d1ee6e0c6c37ab8d9f3549cd419a4f39a2247b71cbb16d7b0b87ec05c289", null ],
      [ "NETMAN_EE_RPC_FUNC_HANDLE_PACKETS", "netman__rpc_8h.html#a2e940d1ee6e0c6c37ab8d9f3549cd419a2c62f869f51f68e82fac559a06d11b23", null ],
      [ "NETMAN_EE_RPC_FUNC_HANDLE_LINK_STATUS_CHANGE", "netman__rpc_8h.html#a2e940d1ee6e0c6c37ab8d9f3549cd419a01db3f1de3d2065eea5ccf7459fb6fbe", null ]
    ] ],
    [ "NETMAN_IOP_RPC_FUNC_NUMS", "netman__rpc_8h.html#aebaac12d50adb621de81d537fd3d9fe4", [
      [ "NETMAN_IOP_RPC_FUNC_INIT", "netman__rpc_8h.html#aebaac12d50adb621de81d537fd3d9fe4a97ea42f44d2970e311a26b9ea62245ac", null ],
      [ "NETMAN_IOP_RPC_FUNC_DEINIT", "netman__rpc_8h.html#aebaac12d50adb621de81d537fd3d9fe4a33daa4d8e69b4b81c617497845c09eec", null ],
      [ "NETMAN_IOP_RPC_FUNC_REG_NETWORK_STACK", "netman__rpc_8h.html#aebaac12d50adb621de81d537fd3d9fe4a82518567f305eba2d3f578d570483dde", null ],
      [ "NETMAN_IOP_RPC_FUNC_UNREG_NETWORK_STACK", "netman__rpc_8h.html#aebaac12d50adb621de81d537fd3d9fe4a84e4a2c6ae71d0c6bfbc1fd61bd9c7d7", null ],
      [ "NETMAN_IOP_RPC_FUNC_IOCTL", "netman__rpc_8h.html#aebaac12d50adb621de81d537fd3d9fe4a712e8165848f6a93b1da4be0c3d42687", null ],
      [ "NETMAN_IOP_RPC_FUNC_SET_MAIN_NETIF", "netman__rpc_8h.html#aebaac12d50adb621de81d537fd3d9fe4a4306bf77b54fc59dae11eec43253b284", null ],
      [ "NETMAN_IOP_RPC_FUNC_QUERY_MAIN_NETIF", "netman__rpc_8h.html#aebaac12d50adb621de81d537fd3d9fe4ae71b329a2064d9734d79bd4e9e6f95ac", null ],
      [ "NETMAN_IOP_RPC_FUNC_SET_LINK_MODE", "netman__rpc_8h.html#aebaac12d50adb621de81d537fd3d9fe4a38f29d8130fd894e8b0be4026767cbec", null ]
    ] ]
];