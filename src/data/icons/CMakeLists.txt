add_subdirectory(actions)

set(kile_ICONS
    hi128-apps-kile.png
    hi16-apps-kile.png
    hi22-apps-kile.png
    hi32-apps-kile.png
    hi48-apps-kile.png
    hi64-apps-kile.png
    hisc-apps-kile.svgz
)

ecm_install_icons(ICONS ${kile_ICONS}
                  DESTINATION ${ICON_INSTALL_DIR})
