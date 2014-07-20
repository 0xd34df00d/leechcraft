<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/">
		#define QT_TRANSLATE_NOOP(a,b)
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="desc">
		QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="text()"/>")
		<xsl:value-of select="document(.)" />
	</xsl:template>
	<xsl:template match="usage">
		QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="text()"/>")
		<xsl:value-of select="document(.)" />
	</xsl:template>
</xsl:stylesheet>

