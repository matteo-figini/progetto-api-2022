# Progetto API 2022
Progetto di _Prova Finale_ del corso di _Algoritmi e Principi dell'Informatica_ (A. A. 2021-2022), Politecnico di Milano

The main goal of this project was to create a C code that satisfied the requirements reported in the "ProvaFinale2022-tema.pdf" file. This is a sort of Wordle game where an online verifier tests some functional and non-functional constraints, especially time and memory limitations, where a very long file is submitted in input (with thousands and thousands of commands).
To optimize the code I decided to use several data structures that allowed me to ease the limitations concerning the algorithm, indeed I used a standard algorithm to compare an input word with the correct one and return a string indicating, for each position, if the character is correct, if it is in the string but in another position or if it is not in the word.

About the data structures, I used two hash tables: one containing all the words (let's call it A) and the other one empty (let's call it B). During a match, all the non-valid words that don't satisfy the constraints learnt until that moment are moved from A to B and an integer value contains the number of the words stored in each hash table. Heuristically, since at the end of the match B is generally much bigger than A, at the end of the first match the role of each table is inverted: all the remaining words in A are moved to B until A is empty, then at the second match words that don't satisfy the constraints are moved from B to A and so on.

Every time a "+stampa_filtrate" command is called, a Red-Black Tree is built with the words in the "active" hash table, then printed and destroyed. Although this doesn't seem to be the best solution, it works well since when the "+stampa_filtrate" command is called the active table is smaller with respect to the moment the match started, and so it seemed to be better to manipulate an O(1) data structure containing the words and build the tree "on the fly", when requested.

Contact me for any further information or inquiries.
