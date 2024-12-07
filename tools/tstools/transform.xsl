<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:strip-space elements="*" />
	<xsl:template match="/">
		#define QT_TRANSLATE_NOOP(a,b)
		<xsl:apply-templates/>
	</xsl:template>

  <xsl:template match="label[@translatable='false']"/>
	<xsl:template match="*[@label or @suffix]">
    <xsl:if test="@label">
      QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="@label"/>")
    </xsl:if>
    <xsl:if test="@suffix">
      QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="@suffix"/>")
    </xsl:if>
		<xsl:apply-templates/>
	</xsl:template>
	<xsl:template match="label[@value]">
		QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="@value"/>")
	</xsl:template>
	<xsl:template match="suffix[@value]">
		QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="@value"/>")
	</xsl:template>
	<xsl:template match="label">
		QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="text()"/>")
	</xsl:template>
	<xsl:template match="suffix">
		QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="text()"/>")
	</xsl:template>

	<xsl:template match="tooltip">
		QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="text()" disable-output-escaping="yes" />")
	</xsl:template>
	<xsl:template match="default">
		QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="text()" disable-output-escaping="yes" />")
	</xsl:template>

	<xsl:template match="item[@translatable='true']">
		QT_TRANSLATE_NOOP("__FILENAME__", "<xsl:value-of select="@default"/>")
		<xsl:apply-templates/>
	</xsl:template>
</xsl:stylesheet>
