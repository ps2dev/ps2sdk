var group__packet2__types =
[
    [ "dma_tag_t", "group__packet2__types.html#structdma__tag__t", [
      [ "QWC", "group__packet2__types.html#a377674b9e78b2e038b15273f02d5c5bf", null ],
      [ "PCE", "group__packet2__types.html#a37176bf12b1c94821f98a7878b8fb26c", null ],
      [ "ID", "group__packet2__types.html#a4862c9e86250ca2ec5e62758c8318efc", null ],
      [ "IRQ", "group__packet2__types.html#a0362fdd47abcf52a5de56327686ee2d2", null ],
      [ "ADDR", "group__packet2__types.html#ac17432d7df9536ba931637e81c798bb4", null ],
      [ "SPR", "group__packet2__types.html#a231f31b2899ce3e7ae70fdbf030b9b40", null ]
    ] ],
    [ "vif_code_t", "group__packet2__types.html#structvif__code__t", [
      [ "immediate", "group__packet2__types.html#a534c6393fed79e9b98f2948130b58b6e", null ],
      [ "num", "group__packet2__types.html#aa193825a4f97a33f7324d50c642bf342", null ],
      [ "cmd", "group__packet2__types.html#a2e5cc7e42aa607f3e97973f7ecabab21", null ]
    ] ],
    [ "packet2_t", "group__packet2__types.html#structpacket2__t", [
      [ "__attribute__", "group__packet2__types.html#aa1044dde0d90113ab8c1d366fdb2a4e5", null ],
      [ "max_qwords_count", "group__packet2__types.html#ae007c7262262112505cc8ce708e58114", null ],
      [ "type", "group__packet2__types.html#a789e15311ca9f946a9fcbfadbbd005d4", null ],
      [ "mode", "group__packet2__types.html#a565131440d20c5fb21fd38feea7ac1ae", null ],
      [ "tte", "group__packet2__types.html#a2ff12e8f90a20e119dd08442223f1bce", null ],
      [ "next", "group__packet2__types.html#af4f4781fa2e353f56492e45eb6ec55e4", null ],
      [ "tag_opened_at", "group__packet2__types.html#add12bb832df6d1cd8cb8b092aada0bf2", null ],
      [ "vif_code_opened_at", "group__packet2__types.html#a5445ab7cf8d8187e4b1355f5355abd6e", null ]
    ] ],
    [ "Mask", "group__packet2__types.html#union_mask", null ],
    [ "Mask.__unnamed69__", "group__packet2__types.html#struct_mask_8____unnamed69____", null ],
    [ "Packet2Mode", "group__packet2__types.html#ga95c5e6780caea137c284750393aed77a", null ],
    [ "Packet2Type", "group__packet2__types.html#gadfa40ef77d518badca76f0b483b32469", [
      [ "P2_TYPE_NORMAL", "group__packet2__types.html#ggadfa40ef77d518badca76f0b483b32469ad134718a715e249512af4e3461be1af1", null ],
      [ "P2_TYPE_UNCACHED", "group__packet2__types.html#ggadfa40ef77d518badca76f0b483b32469a650f24ce34203d4b6b8f8063e9db564e", null ],
      [ "P2_TYPE_UNCACHED_ACCL", "group__packet2__types.html#ggadfa40ef77d518badca76f0b483b32469a073e1acd35178b192979ea4c74b032a1", null ],
      [ "P2_TYPE_SPRAM", "group__packet2__types.html#ggadfa40ef77d518badca76f0b483b32469a9f444e8d514d86a3d37903713cb74836", null ]
    ] ],
    [ "DmaTagType", "group__packet2__types.html#ga467c950bab324b489b9490cbc1457b4f", [
      [ "P2_DMA_TAG_REFE", "group__packet2__types.html#gga467c950bab324b489b9490cbc1457b4fafcb1aefaebe06e8a13ebb9041b330229", null ],
      [ "P2_DMA_TAG_CNT", "group__packet2__types.html#gga467c950bab324b489b9490cbc1457b4fa48e897d5707cb97630a0f2a02ffc1551", null ],
      [ "P2_DMA_TAG_NEXT", "group__packet2__types.html#gga467c950bab324b489b9490cbc1457b4fa4068969e04aa1687d56aacb5d647debd", null ],
      [ "P2_DMA_TAG_REF", "group__packet2__types.html#gga467c950bab324b489b9490cbc1457b4fa67c637ee7a6ab0dd1e16bfb6724abad1", null ],
      [ "P2_DMA_TAG_REFS", "group__packet2__types.html#gga467c950bab324b489b9490cbc1457b4fae28c2323aa5f379f1020b4097af861ad", null ],
      [ "P2_DMA_TAG_CALL", "group__packet2__types.html#gga467c950bab324b489b9490cbc1457b4fa52227631cf144d0a58b83664b553b2e9", null ],
      [ "P2_DMA_TAG_RET", "group__packet2__types.html#gga467c950bab324b489b9490cbc1457b4fa503d493638e9b84d6e943ae758e25cf3", null ],
      [ "P2_DMA_TAG_END", "group__packet2__types.html#gga467c950bab324b489b9490cbc1457b4faecb3f1cee917382e679d08fcd3da4f92", null ]
    ] ],
    [ "UnpackMode", "group__packet2__types.html#ga2be3ff1b71bcdb54096b5ef45a3c4dd3", null ],
    [ "VIFOpcode", "group__packet2__types.html#gaae2dd075c3094f9eb68d5422fd8b99cb", [
      [ "P2_VIF_NOP", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba5ee22087c471fda79e77b02b91e48f01", null ],
      [ "P2_VIF_STCYCL", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba64ac32bdf33505a928a0354fdbaad080", null ],
      [ "P2_VIF_OFFSET", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba50f44d4b806f7f7444ec70437a2f7da7", null ],
      [ "P2_VIF_BASE", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cbae21adf8b6695d78bc56b1d7309393c46", null ],
      [ "P2_VIF_ITOP", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba7f9be6b913c8f031c57c16354fb906c6", null ],
      [ "P2_VIF_STMOD", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba67b7d3513d534f6a5a2a3d1efea99f19", null ],
      [ "P2_VIF_MSKPATH3", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba122784882c1135ef9d78a84a5478cd03", null ],
      [ "P2_VIF_MARK", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cbaf1ad3e2a012950766500b4e1c4f68d8f", null ],
      [ "P2_VIF_FLUSHE", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba5851536e8cc7b1c3f85f0a5b6090b113", null ],
      [ "P2_VIF_FLUSH", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba5cd3da29b635a703e2bb4a312d737f81", null ],
      [ "P2_VIF_FLUSHA", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cbaeb4f1c1bd5a5f34f8ac35fb3d9303a11", null ],
      [ "P2_VIF_MSCAL", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba45f3852b8dc5328930f6b753dc8686b5", null ],
      [ "P2_VIF_MSCNT", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba88437f33e764d68ceca57eecc1e190b4", null ],
      [ "P2_VIF_MSCALF", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba52dd6117fbd42999032e258e7bd9e2f4", null ],
      [ "P2_VIF_STMASK", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba2e9161362bdc390fdbd8171c295274d5", null ],
      [ "P2_VIF_STROW", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba151a3e4d561d5a05f7840c87547860ef", null ],
      [ "P2_VIF_STCOL", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba4de8151c4a4a33b80574a3948905561c", null ],
      [ "P2_VIF_MPG", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba1a768becb4d7b9024788eb3b7c393035", null ],
      [ "P2_VIF_DIRECT", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cbaaccf5935fa66e3093eaeb338f216e9d7", null ],
      [ "P2_VIF_DIRECTHL", "group__packet2__types.html#ggaae2dd075c3094f9eb68d5422fd8b99cba6bccbc1e2534bb738ec63555d2c82080", null ]
    ] ]
];