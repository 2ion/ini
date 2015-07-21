ini.1: ini.1.mkd
	@PANDOC@ -s -t man $< -o $@

man1_MANS = ini.1

mostlyclean-local:
	-rm $(man1_MANS)

EXTRA_DIST = $(man1_MANS)
