#!/usr/bin/php
<?php
// En-tête CGI pour indiquer le type de contenu
header("Content-Type: text/html");

// Chemin vers le fichier HTML
$htmlFile = './site/test.html';

// Récupération de la police depuis le formulaire POST
$font = isset($_POST['font']) ? $_POST['font'] : 'Doto';

// Charger le contenu du fichier HTML
$htmlContent = file_get_contents($htmlFile);

// Générer le style CSS pour la police choisie
$styleTag = "<style>body { font-family: $font; }</style>";

// Remplacer ou ajouter le bloc <style> pour la police
if (preg_match('/<style>.*?<\/style>/s', $htmlContent)) {
    // Remplacer un bloc <style> existant
    $htmlContent = preg_replace('/<style>.*?<\/style>/s', $styleTag, $htmlContent);
} else {
    // Ajouter le bloc <style> avant la fin de <head>
    $htmlContent = preg_replace('/(<\/head>)/', $styleTag . "\n" . '$1', $htmlContent);
}

// Sauvegarder le contenu modifié dans le fichier HTML
file_put_contents($htmlFile, $htmlContent);
?>
