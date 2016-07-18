# Joint Event and Entity Extraction
Implementation of the joint event and entity models described in "Joint Extraction of Events and Entities within a Document Context" by Bishan Yang and Tom M. Mitchell.

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

## License

## References
CRF++: Yet Another CRF toolkit (https://taku910.github.io/crfpp/)

AD3 (approximate MAP decoder with Alternating Direction Dual Decomposition) (https://github.com/andre-martins/AD3)

```latex
@InProceedings{yang2016joint,
  author    = {Yang, Bishan and Mitchell, Tom},
  title     = {Joint Extraction of Events and Entities within a Document Context},
  booktitle = {Proceedings of the 2016 Conference of the North American Chapter of the Association for Computational Linguistics: Human Language Technologies},
  year      = {2016},
  pages     = {289--299}
}
