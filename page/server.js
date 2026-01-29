const express = require('express');
const app = express();
const path = require('path');

const PORT = 3000;

// Servir archivos estáticos desde la carpeta 'public'
app.use(express.static(path.join(__dirname, 'public')));

// Ruta principal
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.listen(PORT, () => {
    console.log(`\n🚀 Servidor corriendo!`);
    console.log(`👉 Abre tu navegador en: http://localhost:${PORT}`);
    console.log(`(Presiona Ctrl + C para detenerlo)\n`);
});