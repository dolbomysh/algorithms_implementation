from langchain_community.document_loaders import TextLoader
from langchain.text_splitter import RecursiveCharacterTextSplitter
from langchain_community.embeddings import HuggingFaceEmbeddings
from langchain_community.vectorstores import FAISS

loader = TextLoader("task8_text.txt", encoding="utf-8")
docs = loader.load()

splitter = RecursiveCharacterTextSplitter(chunk_size=500, chunk_overlap=0)
chunks = splitter.split_documents(docs)

embeddings = HuggingFaceEmbeddings(model_name="sentence-transformers/all-MiniLM-L6-v2")

db = FAISS.from_documents(chunks, embeddings)

query = "How do frogs breathe?"

results = db.similarity_search(query)

print(results[0].page_content)

# Вывод
# Frogs have three-chambered hearts, a feature they share with lizards.
# Oxygenated blood from the lungs and de-oxygenated blood from the respiring tissues enter the heart through separate atria.
# When these chambers contract, the two blood streams pass into a common ventricle before being pumped via a spiral valve to the appropriate vessel,
# the aorta for oxygenated blood and pulmonary artery for deoxygenated blood.[73]
