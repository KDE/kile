add_subdirectory(actions)

set(kile_ICONS
	16-apps-kile.png
	22-apps-kile.png
	32-apps-kile.png
	44-apps-kile.png
	48-apps-kile.png
	64-apps-kile.png
	128-apps-kile.png
	150-apps-kile.png
	310-apps-kile.png
	sc-apps-kile.svg
)

ecm_install_icons(
	ICONS ${kile_ICONS}
	DESTINATION ${ICON_INSTALL_DIR}
)
