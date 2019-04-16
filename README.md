Simple regex implementation.
Regex input must consist of ASCII symbols and operators '(', ')', '*' and '|'.

Implementation uses recursive-descendent parser to build syntax tree from regex. This syntax tree next used to evaluate followpos of each node symbol (pos) in regex. Next, we create Determenistic Finite Automaton and use it to match expressions.

Method used: 3.9.1-3.9.4, Aho. Compilers - Principles, Techniques, and Tools 2e