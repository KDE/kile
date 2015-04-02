add_subdirectory(actions)

set(kile_ICONS
	128-apps-kile.png
	16-apps-kile.png
	22-apps-kile.png
	32-apps-kile.png
	48-apps-kile.png
	64-apps-kile.png
	sc-apps-kile.svgz
)

ecm_install_icons(
	ICONS ${kile_ICONS}
	DESTINATION ${ICON_INSTALL_DIR}
)
