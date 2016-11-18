@ss = (
	'atk',
	'bzip2',
	'cairo',
	'libepoxy',
	'expat',
	'libffi',
	'fontconfig',
	'freetype',
	'gcc',
	'gdk-pixbuf2',
	'gtk3',
	'glib2',
	'graphite2',
	'gsl',
	'harfbuzz',
	'libiconv',
	'gettext',
	'json-glib',
	'xz',
	'pango',
	'pcre',
	'pixman',
	'libpng',
	'libxml2',
	'zlib');

foreach $s (@ss)
{
	$url = "https://raw.githubusercontent.com/Alexpux/MINGW-packages/master/mingw-w64-".$s."/PKGBUILD";
	@ls = split(/[\r\n]+/, `wget -qO- '$url'`);
	%var = ();
	foreach $l (@ls)
	{
		$l =~ s/\s+//g;
		if($l=~ /^([^\=]+)\=(.*)/)
		{
			$var{$1} = $2;
		}
	}
	if($var{"source"} =~ /(http|ftp)/)
	{

		if ($s =~ /cairo/)
		{
			$var{"source"} =~ s/release/snapshot/g;
		}

		$var{"source"} =~ s/^\(//g;
		$var{"source"} =~ s/^\#//g;
		$var{"source"} =~ s/^\"//g;
		$var{"source"} =~ s/\)$//g;
		$var{"source"} =~ s/\#$//g;
		$var{"source"} =~ s/\{,\.sig\}$//g;
		$var{"source"} =~ s/\"$//g;
		$var{"source"} =~ s/^.*?(http|ftp)/$1/g;

		$pkgver = $var{"pkgver"};
		$pkgver =~ s/^([^\.]+\.[^\.]+)\..*/$1/g;
		$var{"source"} =~ s/\$\{(pkgver[^\}]+)\}/$pkgver/g;

		$var{"source"} =~ s/\$\{([^\}]+)\}/if(exists $var{$1}){$var{$1}}else{'UNDEF'}/eg;

		print $s."\t".$var{"source"}."\n";

		system "wget -q ".$var{"source"};
	}
}
