# Joint Event and Entity Extraction
Implementation of the joint event and entity model described in "Joint Extraction of Events and Entities within a Document Context" by Bishan Yang and Tom M. Mitchell.

If you are using this code in your work, please cite the following publication:

Bishan Yang and Tom Mitchell (2016). Joint Extraction of Events and Entities within a Document Context. Proceedings of the 2016 Conference of the North American Chapter of the Association for Computational Linguistics: Human Language Technologies (NAACL), pp. 289-299

For any questions/issues about the code, please feel free to email Bishan Yang (bishan.yang@gmail.com).

```latex
@InProceedings{yang2016joint,
  author    = {Yang, Bishan and Mitchell, Tom},
  title     = {Joint Extraction of Events and Entities within a Document Context},
  booktitle = {Proceedings of the 2016 Conference of the North American Chapter of the Association for Computational Linguistics: Human Language Technologies},
  year      = {2016},
  pages     = {289--299}
}
```

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

## License
GNU Lesser General Public License v3.0
