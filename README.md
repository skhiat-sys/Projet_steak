# Stéganographie BMP



## I. Installation et Exécution

### 1. Installation

##### Aucune installation particulière n’est requise :

##### Téléchargez l’exécutable fourni ou compilez le projet avec Visual Studio/MinGW.

##### Aucune bibliothèque supplémentaire n’est nécessaire : GDI+ est déjà intégré à Windows.

##### Si vous compilez, l’ajout de `gdiplus.lib` est requis.

### 2. Exécution

##### Lancez l’application en double-cliquant sur *StegApp.exe*

##### Ou exécutez-la depuis Visual Studio après compilation (Ctrl + F5)




## II. Formats supportés

### Formats affichables

#####  BMP

### Formats utilisables pour la stéganographie

#### *BMP uniquement (24 bits) :*

##### La méthode LSB nécessite un accès direct aux octets du fichier BMP.
##### Exemple de récupération de l’offset du pixel data :

`int offset = *reinterpret_cast<int*>(&bmpData[10]);`
`// Le pixel data commence généralement à l’offset 54`

##### Ce format est indispensable car l’intégration utilise une écriture directe dans les octets du fichier.




## III. Procédure d’insertion (Chiffrement)

### Étape 1 — Ouvrir une image

##### Menu Fichier

##### Cliquer Ouvrir une image

##### Sélectionner un fichier BMP

#### *L'image est chargée en mémoire via :*

`vector<unsigned char> data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());`


### Étape 2 — Entrer le texte

##### Saisir le message à cacher dans la zone de texte située en bas de la fenêtre.

### Étape 3 — Chiffrer 

##### Menu Stéganographie

##### Cliquer Chiffrer

#### *L’intégration LSB s’effectue ainsi :*

`bmpData[dataIndex] &= 0xFE;           // Efface le dernier bit`

`bmpData[dataIndex] |= ((c >> i) & 1); // Insère un bit du message`

##### Un octet `0` est ajouté à la fin pour indiquer la fin du message.

#### L’image modifiée est enregistrée sous output.bmp et affichée immédiatement.




## IV. Procédure d’extraction (Déchiffrement)

### Étape 1 — Ouvrir l'image modifiée

##### Menu Fichier → Ouvrir une image

##### Sélectionner output.bmp ou tout autre BMP contenant un message LSB.

### Étape 2 — Déchiffrer

##### Menu Stéganographie

##### Cliquer Déchiffrer

#### *Le texte est reconstruit bit par bit :*

    c |= ((bmpData[dataIndex] & 1) << bitCount);

    if (bitCount == 8) {

        if (c == 0) break;   // fin du message détectée
    
        message += c;
    }
#### Le message caché apparaît dans la zone de texte.
