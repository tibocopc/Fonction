# Calculateur de Dérivées Symboliques

Programme C pour le calcul automatique de dérivées symboliques d'expressions mathématiques.

## Fonctionnalités

- **Opérateurs supportés**: `+`, `-`, `*`, `/`, `^` (puissance)
- **Fonctions supportées**: `sin`, `cos`, `exp`, `ln`
- **Variables**: Toutes les lettres (x, u, v, z, etc.)
- **Simplification automatique**: Les résultats sont simplifiés (élimination des zéros, des uns, etc.)

## Compilation

```bash
make
```

Nécessite `gcc` et les bibliothèques mathématiques standard.

## Utilisation

```bash
./derivative
```

Le programme demande une expression mathématique en entrée et affiche sa dérivée symbolique par rapport à la variable `x`.

### Exemples

```
Entrez une fonction: x^2*sin(x)
Expression: x^2*sin(x)
Dérivée d/dx: 2*x*sin(x)+x^2*cos(x)
```

```
Entrez une fonction: x^3
Expression: x^3
Dérivée d/dx: 3*x^2
```

```
Entrez une fonction: exp(x^2)
Expression: exp(x^2)
Dérivée d/dx: exp(x^2)*2*x
```

```
Entrez une fonction: sin(x)*cos(x)
Expression: sin(x)*cos(x)
Dérivée d/dx: cos(x)*cos(x)+sin(x)*-1*sin(x)
```

```
Entrez une fonction: ln(x)/x
Expression: ln(x)/x
Dérivée d/dx: (1/x*x-ln(x))/(x^2)
```

## Tests automatiques

Pour exécuter une suite de tests prédéfinis:

```bash
make test
```

## Architecture

Le programme est structuré en plusieurs modules:

1. **Lexeur**: Tokenise l'expression en entrée
2. **Parseur**: Construit un arbre d'expression à partir des tokens (analyse syntaxique)
3. **Dérivation**: Applique les règles de dérivation symbolique
4. **Simplification**: Simplifie l'expression résultante
5. **Affichage**: Convertit l'arbre en notation mathématique lisible

### Règles de dérivation implémentées

- Constante: `d/dx(c) = 0`
- Variable: `d/dx(x) = 1`
- Somme: `d/dx(f + g) = f' + g'`
- Différence: `d/dx(f - g) = f' - g'`
- Produit: `d/dx(f * g) = f' * g + f * g'`
- Quotient: `d/dx(f / g) = (f' * g - f * g') / g²`
- Puissance: `d/dx(f^n) = n * f^(n-1) * f'` (si n est constant)
- Sinus: `d/dx(sin(f)) = cos(f) * f'`
- Cosinus: `d/dx(cos(f)) = -sin(f) * f'`
- Exponentielle: `d/dx(exp(f)) = exp(f) * f'`
- Logarithme: `d/dx(ln(f)) = f' / f`

## Nettoyage

```bash
make clean
```

## Limitations

- La dérivation se fait toujours par rapport à la variable `x`
- Les expressions doivent être syntaxiquement correctes
- Pas de support pour les fonctions trigonométriques inverses (arcsin, arccos, etc.)