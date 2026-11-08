includes("lib/commonlibsse-ng")


set_project("NoKillmoves")
set_version("1.1.0")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

target("NoKillmoves")
    add_rules("commonlibsse-ng.plugin", {
        name = "NoKillmoves",
        author = "Rain",
        description = "Switch kill moves off for the player and NPCs, the kill move camera, and magic / bow kill cams"
    })

    add_installfiles("dist/NoKillmoves.ini", { prefixdir = "SKSE/Plugins" })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    add_includedirs("extern/SKSEMenuFramework")
    set_pcxxheader("src/pch.h")
