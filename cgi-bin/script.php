#!/usr/bin/php
<?php
// En-tête CGI pour indiquer le type de contenu
header("Content-Type: text/html");

// Chemin vers le fichier HTML
$htmlFile = './site/test.html';

// Liste de polices parmi lesquelles choisir
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

// Sélection d'une police aléatoire
$font = $fonts[array_rand($fonts)];

// Charger le contenu du fichier HTML
$htmlContent = file_get_contents($htmlFile);

// Générer le style CSS pour la balise <h1> avec la police choisie
$styleTag = "h1 { font-family: $font; }";

// Vérifier si un style pour <h1> existe déjà
if (preg_match('/h1\s*{[^}]*}/', $htmlContent)) {
    // Remplacer le style existant pour <h1> avec la nouvelle police
    $htmlContent = preg_replace('/h1\s*{[^}]*}/', $styleTag, $htmlContent);
} else {
    // Ajouter le bloc <style> avec le style <h1> avant </head> si aucun style <h1> n'existe
    if (strpos($htmlContent, '<style>') !== false) {
        // Ajouter dans le bloc <style> existant
        $htmlContent = preg_replace('/(<\/style>)/', "$styleTag\n$1", $htmlContent);
    } else {
        // Ajouter un nouveau bloc <style> avant </head>
        $htmlContent = preg_replace('/(<\/head>)/', "<style>$styleTag</style>\n$1", $htmlContent);
    }
}

// Sauvegarder le contenu modifié dans le fichier HTML
file_put_contents($htmlFile, $htmlContent);
?>
