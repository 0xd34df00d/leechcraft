<xslt:stylesheet version="1.0" xmlns:xslt="http://www.w3.org/1999/XSL/Transform">
	<xslt:output method="text"/>
	<xslt:template match="*">
		<xslt:apply-templates/>
	</xslt:template>
	<xslt:template match="text()"/>
	<xslt:template match="icondef/icon/text">
		<xslt:value-of select="../object"/><xslt:text> </xslt:text><xslt:value-of select="."/>
<xslt:text>
</xslt:text>
	</xslt:template>
</xslt:stylesheet>
