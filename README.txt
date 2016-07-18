==== input ======
ace.test.conll stores the test documents in the CoNLL format:

DocID SentID TokenID Word POS Parse Lemma - - - - Tag

ace.test.dependencies.txt stores the Stanford dependency parse outputs for sentences in the test documents.

ace.test.stanford.ner.txt stores the named entity detected by Stanford NER using the following format:

NER_Type  DocID  SentID  StartOffset,EndOffset


==== output ======
joint.results.txt stores the extracted events, entities, and argument roles.

The outputs are organized according to the detected event triggers. For each trigger, we output its event type, and for each of its associated argument candidate, we output its entity type and role type: 

DocID#SentID#StartOffset#EndOffset  EventType
DocID#SentID#StartOffset#EndOffset#EntityType  RoleType
DocID#SentID#StartOffset#EndOffset#EntityType  RoleType
. . .
