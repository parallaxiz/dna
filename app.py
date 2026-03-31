import streamlit as st  # type: ignore
import streamlit.components.v1 as components  # type: ignore
from pathlib import Path

st.set_page_config(
    page_title="DNA Sequence Analyzer",
    page_icon="🧬",
    layout="wide",
    initial_sidebar_state="collapsed",
)

# Hide default Streamlit chrome for a clean full-screen experience
st.markdown(
    """
    <style>
        #MainMenu, header, footer, [data-testid="stToolbar"] { display: none !important; }
        .block-container { padding: 0 !important; max-width: 100% !important; }
        [data-testid="stAppViewContainer"] { padding-top: 0 !important; }
        iframe { border: none !important; }
    </style>
    """,
    unsafe_allow_html=True,
)

# Load the HTML file and render it as a full-page component
html_path = Path(__file__).parent / "dna_analyzer.html"
html_content = html_path.read_text(encoding="utf-8")
components.html(html_content, height=1200, scrolling=True)