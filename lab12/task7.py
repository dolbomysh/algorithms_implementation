import hashlib
import re
from random import randint
import pandas as pd
from sklearn.datasets import fetch_20newsgroups

data = fetch_20newsgroups(subset='train', remove=('headers', 'footers', 'quotes'))
documents = data.data


class Shingler:
    def __init__(self, k=10):
        self.k = k

    def process_doc(self, document):
        return re.sub(r"( )+|(\n)+", " ", document).lower()

    def get_shingles(self, document):
        document = self.process_doc(document)
        return {document[i:i + self.k] for i in range(len(document) - self.k + 1)}


class HashFamily:
    def __init__(self, seed_value):
        self.result_size = 8
        self.max_len = 20
        self.salt = str(seed_value).zfill(self.max_len)[-self.max_len:]

    def get_hash_value(self, el_to_hash):
        combined = str(el_to_hash).encode('utf-8') + self.salt.encode('utf-8')
        hash_val = hashlib.sha1(combined).hexdigest()[-self.result_size:]
        return int(hash_val, 16)


class MinHashSigner:
    def __init__(self, sig_size):
        self.sig_size = sig_size
        self.hash_functions = [HashFamily(randint(0, int(1e9))) for _ in range(sig_size)]

    def compute_signature(self, shingles):
        return [
            min(hf.get_hash_value(sh) for sh in shingles)
            for hf in self.hash_functions
        ]

    def compute_signature_matrix(self, shingle_sets):
        return [self.compute_signature(s) for s in shingle_sets]


class LSH:
    def __init__(self, threshold=0.8, bands_nr=20):
        self.threshold = threshold
        self.bands_nr = bands_nr

    def _get_bands(self, sig_matrix):
        sign_len = len(sig_matrix[0])
        r = sign_len // self.bands_nr
        bands = {i: [] for i in range(self.bands_nr)}

        for signature in sig_matrix:
            for i in range(self.bands_nr):
                segment = signature[i*r:(i+1)*r]
                bands[i].append(tuple(segment))  # using tuple instead of str
        return bands

    def _get_buckets(self, band, hash_func):
        buckets = {}
        for doc_id, segment in enumerate(band):
            h = hash_func.get_hash_value(segment)
            buckets.setdefault(h, []).append(doc_id)
        return buckets

    def _get_candidates(self, buckets):
        candidates = set()
        for bucket_docs in buckets.values():
            if len(bucket_docs) > 1:
                for i in range(len(bucket_docs)):
                    for j in range(i+1, len(bucket_docs)):
                        candidates.add(tuple(sorted((bucket_docs[i], bucket_docs[j]))))
        return candidates

    def _jaccard_similarity(self, sig1, sig2):
        set1, set2 = set(sig1), set(sig2)
        return len(set1 & set2) / len(set1 | set2)

    def get_similar_items(self, sig_matrix):
        sign_len = len(sig_matrix[0])
        similar_docs = set()
        bands = self._get_bands(sig_matrix)

        for i in range(self.bands_nr):
            hash_func = HashFamily(randint(0, int(1e9)))
            buckets = self._get_buckets(bands[i], hash_func)
            candidates = self._get_candidates(buckets)

            for doc1, doc2 in candidates:
                sim = self._jaccard_similarity(sig_matrix[doc1], sig_matrix[doc2])
                if sim >= self.threshold:
                    similar_docs.add((doc1, doc2))
        return similar_docs



# возьмем датасет quora-question-pairs, содержащий пары похожих вопросов и посмотрим, сможет ли наш хэш их сопоставить

if __name__ == "__main__":

    dataset_name = ("quora-question-pairs/train.csv")

    print("Producing Shingles...")
    print("Loading dataset...")
    dataset = pd.read_csv(dataset_name)[:3000]
    print("Dataset loaded correctly.")


    shingling_list = []
    shingling_size = 10
    signature_size = 50
    bands_nr = 10

    shingler_inst = Shingler(shingling_size)

    for index, row in dataset.iterrows():
        doc1 = str(row['question1'])
        doc2 = str(row['question2'])

        shinglings1 = shingler_inst.get_shingles(doc1)
        shinglings2 = shingler_inst.get_shingles(doc2)

        shingling_list.append(shinglings1)
        shingling_list.append(shinglings2)

    signer = MinHashSigner(signature_size)
    print("Computing signature matrix...")
    signature_matrix = signer.compute_signature_matrix(shingling_list)

    lsh_instance = LSH(threshold=0.3)
    print("Computing LSH similarity...")
    lsh_similar_itemset = lsh_instance.get_similar_items(signature_matrix)

    accuracy = 0

    for first_id, second_id in lsh_similar_itemset:
        if second_id == first_id + 1 and first_id % 2 == 0:
            accuracy+=1
    accuracy /= len(dataset)

    print("Similar Elements Found: %d" % (len(lsh_similar_itemset)))
    print("Accuracy: %f" % (accuracy))
    print("Examples of similar objects :")
    for first_id, second_id in list(lsh_similar_itemset)[0:10]:
        print(f" '{dataset["question1"][first_id // 2]}' and  '{dataset["question2"][second_id // 2]}'")



# Вывод :
# Producing Shingles...
# Loading dataset...
# Dataset loaded correctly.
# Computing signature matrix...
# Computing LSH similarity...
# Similar Elements Found: 555
# Accuracy: 0.100000
# Examples of similar objects :
#  'What is the penalty for driving without a license in Alabama, how do they compare to the penalties in Massachusetts?' and  'What is the penalty for driving without a license in Alabama, how do they compare to the penalties in Maine?'
#  'How do I delete my question from Quora?' and  'How do I delete my own question from Quora?'
#  'What universities does Sigma Designs recruit new grads from? What majors are they looking for?' and  'What universities does Chart Industries recruit new grads from? What majors are they looking for?'
#  'How I become a good teacher?' and  'How do you become a good teacher?'
#  'How do you become a military pilot?' and  'How can I become a military pilot?'
#  'What are some lesser known facts about ants?' and  'Indian Ethnicity and People: What are the interesting facts about sonia gandhi in India?'
#  'How can I unblock my Facebook account on Chrome?' and  'How can I unblock a Facebook account on Chrome?'
#  'What is vitamin N?' and  'What is vitamin G?'
#  'What is the best advice your father ever gave you?' and  'What's the best advice your father ever gave you?'
#  'When a girlfriend asks her boyfriend "Why did you choose me? What makes you want to be with me?", what should one reply to her?' and  'After 2 years being in a relationship my girlfriend says she doesnt want to get physical with me. She says she doesn't like it now. I always think something is wrong and we end up fighting, but she says she loves me. I am confused. What should I do and believe?'
#