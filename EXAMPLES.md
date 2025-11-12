# Exemples de tests pour le calculateur de dérivées

## Exemples de base

### Polynômes
- x^2 → 2*x
- x^3 → 3*x^2
- x^5 → 5*x^4
- x^2+3*x+5 → 2*x+3

### Fonctions trigonométriques
- sin(x) → cos(x)
- cos(x) → -1*sin(x)
- sin(x)*cos(x) → cos(x)*cos(x)+sin(x)*-1*sin(x)

### Exponentielle et logarithme
- exp(x) → exp(x)
- ln(x) → 1/x
- exp(x^2) → exp(x^2)*2*x
- ln(x)/x → (1/x*x-ln(x))/(x^2)

### Cas complexe (exemple principal)
- x^2*sin(x) → 2*x*sin(x)+x^2*cos(x)

### Produits et quotients
- x*exp(x) → exp(x)+x*exp(x)
- (x+1)*(x+2) → x+2+x+1 (= 2*x+3)

## Usage

Pour tester chaque exemple:
```bash
echo "x^2*sin(x)" | ./derivative
```

Ou interactivement:
```bash
./derivative
```
