FastWER
====

A PyPI package for fast word/character error rate (WER/CER) calculation
* fast (cpp implementation)
* sentence-level and corpus-level WER/CER scores

It was also extended with new functionalities:
* Computation of Bag of words
* Support for UTF.8 encoding
* Traceback algorithm implementation for obtaining the aligning-path
through a minimum edit-distance between reference and hypothesis strings


# Installation
```bash
pip install pybind11 fastwer
```

# Example
```python
import fastwer
hypo = ['This is an example .', 'This is another example .']
ref = ['This is the example :)', 'That is the example .']

# Corpus-Level WER: 40.0
fastwer.score(hypo, ref)
# Corpus-Level CER: 25.5814
fastwer.score(hypo, ref, char_level=True)

# Sentence-Level WER: 40.0
fastwer.score_sent(hypo[0], ref[0])
# Sentence-Level CER: 22.7273
fastwer.score_sent(hypo[0], ref[0], char_level=True)
```


# Contact
* Changhan Wang (wangchanghan@gmail.com)
* For extensions: Alejandro H. Toselli (ahector@prhlt.upv.es)
