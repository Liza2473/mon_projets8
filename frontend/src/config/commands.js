// ✅ Catalogue centralisé de toutes les commandes possibles
// Le client choisit lesquelles afficher dans le panneau Configurer
// L'aide est generee automatiquement a partir de cette liste

export const ALL_COMMANDS = [
  {
    id: 'LIST',
    label: 'Liste les flux',
    desc: 'Affiche la liste de tous les flux disponibles sur l\'appareil',
    category: 'Global'
  },
  {
    id: 'ALL',
    label: 'Affiche tout',
    desc: 'Affiche toutes les valeurs disponibles en une fois',
    category: 'Global'
  },
  {
    id: 'STATUS',
    label: 'État des flux',
    desc: 'Affiche l\'état (OK/N-A) de chaque flux disponible',
    category: 'Global'
  },
  {
    id: 'TEMP',
    label: 'Température',
    desc: 'Affiche la valeur de température mesurée',
    category: 'Capteur'
  },
  {
    id: 'TEMP?',
    label: 'Température existe ?',
    desc: 'Demande à l\'appareil si le capteur de température est disponible',
    category: 'Question'
  },
  {
    id: 'PRESS',
    label: 'Pression',
    desc: 'Affiche la valeur de pression atmosphérique mesurée',
    category: 'Capteur'
  },
  {
    id: 'PRESS?',
    label: 'Pression existe ?',
    desc: 'Demande à l\'appareil si le capteur de pression est disponible',
    category: 'Question'
  },
  {
    id: 'HUMIDITY',
    label: 'Humidité',
    desc: 'Affiche le taux d\'humidité mesuré',
    category: 'Capteur'
  },
  {
    id: 'HUMIDITY?',
    label: 'Humidité existe ?',
    desc: 'Demande à l\'appareil si le capteur d\'humidité est disponible',
    category: 'Question'
  },
  {
    id: 'BATTERY',
    label: 'Batterie',
    desc: 'Affiche le niveau de batterie de l\'appareil',
    category: 'Capteur'
  },
  {
    id: 'BATTERY?',
    label: 'Batterie existe ?',
    desc: 'Demande à l\'appareil s\'il dispose d\'un capteur de batterie',
    category: 'Question'
  },
  {
    id: 'LUMINOSITE',
    label: 'Luminosité',
    desc: 'Affiche le niveau de luminosité mesuré',
    category: 'Capteur'
  },
  {
    id: 'LUMINOSITE?',
    label: 'Luminosité existe ?',
    desc: 'Demande à l\'appareil si le capteur de luminosité est disponible',
    category: 'Question'
  },
  {
    id: 'CO2',
    label: 'CO2',
    desc: 'Affiche le taux de CO2 mesuré',
    category: 'Capteur'
  },
  {
    id: 'CO2?',
    label: 'CO2 existe ?',
    desc: 'Demande à l\'appareil si le capteur de CO2 est disponible',
    category: 'Question'
  },
  {
    id: 'GAZ',
    label: 'Gaz',
    desc: 'Affiche le niveau de gaz détecté',
    category: 'Capteur'
  },
  {
    id: 'GAZ?',
    label: 'Gaz existe ?',
    desc: 'Demande à l\'appareil si le capteur de gaz est disponible',
    category: 'Question'
  }
]

// ✅ Commandes affichées par défaut si le client n'a rien configuré
export const DEFAULT_COMMANDS = ['LIST', 'TEMP', 'PRESS', 'STATUS', 'TEMP?', 'PRESS?', 'ALL']