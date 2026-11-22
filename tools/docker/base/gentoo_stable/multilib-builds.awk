BEGIN { RS = ""; FS = "\n" }
{
	cpv = id = ""
	multilib = 0
	for (i = 1; i <= NF; ++i)
		if ($i ~ /^CPV: /)
			cpv = substr($i, 6)
		else if ($i ~ /^BUILD_ID: /)
			id = substr($i, 11)
		else if ($i ~ /^USE: / && ($i " ") ~ / abi_x86_32 /)
			multilib = 1
	if (multilib && cpv != "" && id != "")
		print "=" cpv "-" id
}
