    JEE (a joint event and entity extractor for English)
    Copyright (C) 2016
    Bishan Yang and Tom M. Mitchell
    Machine Learning Department, Carnegie Mellon University
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
# Joint Event and Entity Extraction
Implementation of the joint event and entity model described in "Joint Extraction of Events and Entities within a Document Context" by Bishan Yang and Tom M. Mitchell.

## Compile the code
cd Release

make clean

make

## Run the model
./Release/JEE

## Example input
ace.test.conll: test documents in the CoNLL format

ace.test.dependencies.txt: the Stanford dependency parse outputs 

ace.test.stanford.ner.txt: the Stanford NER outputs

NER_Type  DocID  SentID  StartOffset,EndOffset

## References
CRF++: Yet Another CRF toolkit (https://taku910.github.io/crfpp/)

AD3 (approximate MAP decoder with Alternating Direction Dual Decomposition) (https://github.com/andre-martins/AD3)

Stanford CoreNLP version 3.6.0 (http://stanfordnlp.github.io/CoreNLP/)

```latex
@InProceedings{yang2016joint,
  author    = {Yang, Bishan and Mitchell, Tom},
  title     = {Joint Extraction of Events and Entities within a Document Context},
  booktitle = {Proceedings of the 2016 Conference of the North American Chapter of the Association for Computational Linguistics: Human Language Technologies},
  year      = {2016},
  pages     = {289--299}
}
