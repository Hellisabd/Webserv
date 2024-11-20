#!/usr/bin/php
<?php
header("Content-Type: text/html");

$fonts = [
    "Arial, sans-serif",
    "'Times New Roman', Times, serif",
    "Georgia, serif",
    "Courier New, monospace",
    "Verdana, sans-serif",
    "Tahoma, sans-serif",
    "'Lucida Console', Monaco, monospace",
    "'Comic Sans MS', cursive, sans-serif"
];

$font = $fonts[array_rand($fonts)];
$htmlContent = file_get_contents('./site/cgi.html');
$styleTag = "h1 { font-family: $font; }";
$htmlContent = preg_replace('/(<\/head>)/', "<style>$styleTag</style>\n$1", $htmlContent);
print($htmlContent);
?>
