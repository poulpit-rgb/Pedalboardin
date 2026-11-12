# Pédalier

Pedalboard **virtuel** pour jouer en live : la guitare entre dans la carte son, passe dans
des pédales d'effets dessinées à l'écran, et un pédalier MIDI (par exemple un
Hotone Ampero Control) allume ou éteint une ou plusieurs pédales d'un seul appui.

Logiciel libre sous licence **AGPL v3**, écrit en C++ avec [JUCE](https://juce.com).

## Feuille de route

1. **Fondations** *(en cours)* : banc d'essai qui vérifie la carte son, le délai et le pédalier MIDI.
2. **Le son et les premières pédales** : saturation, delay, reverb, chorus, accordeur.
3. **Un switch, plusieurs pédales** : apprentissage MIDI et presets par morceau.
4. **Belles pédales et mode scène** : interface soignée, affichage plein écran, tests de fiabilité.

## Le banc d'essai (étape 1)

L'application *Pédalier - Banc d'essai* permet de :

- choisir la carte son (ex. Focusrite Scarlett 6i6) et sa taille de buffer ;
- entendre la guitare passer directement de l'entrée à la sortie, sans effet ;
- lire le **délai aller-retour estimé** (objectif : moins de 10 ms) ;
- voir un vumètre d'entrée qui passe au rouge si le signal sature ;
- afficher chaque message envoyé par les switchs du pédalier MIDI (CC, Program Change, notes).

Branche le pédalier MIDI **avant** de lancer l'application.

### Télécharger (Windows)

Chaque version compilée est disponible dans l'onglet **Actions** du dépôt GitHub
(artefact `Pedalier-Banc-d-essai-Windows`).

### Compiler soi-même

Il faut CMake 3.22+ et un compilateur C++20 (Visual Studio 2022 sous Windows, GCC ou Clang sous Linux).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

JUCE est téléchargé automatiquement. Pour utiliser une copie locale : `-DJUCE_DIR=/chemin/vers/JUCE`.

Sous Linux, installer d'abord :
`libasound2-dev libfreetype-dev libfontconfig1-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxcomposite-dev libxext-dev`.

**ASIO (Windows, optionnel)** : WASAPI marche directement. Pour le délai le plus bas avec une
Focusrite, télécharge le SDK ASIO de Steinberg et ajoute `-DASIO_SDK_DIR=C:/chemin/vers/asiosdk`.
