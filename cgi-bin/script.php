#!/usr/bin/php
<?php
header("Content-Type: text/html");

$htmlFile = './site/test.html';

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

$htmlContent = file_get_contents($htmlFile);

$styleTag = "h1 { font-family: $font; }";

if (preg_match('/h1\s*{[^}]*}/', $htmlContent)) {
    $htmlContent = preg_replace('/h1\s*{[^}]*}/', $styleTag, $htmlContent);
} else {
    if (strpos($htmlContent, '<style>') !== false) {
        $htmlContent = preg_replace('/(<\/style>)/', "$styleTag\n$1", $htmlContent);
    } else {
        $htmlContent = preg_replace('/(<\/head>)/', "<style>$styleTag</style>\n$1", $htmlContent);
    }
}

file_put_contents($htmlFile, $htmlContent);
?>
