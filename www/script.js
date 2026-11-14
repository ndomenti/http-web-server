(function () {
    'use strict';

    // Add or remove entries here and the page updates itself.
    var IMAGES = [
        { label: '20 MB', path: 'png/20MB.png' },
        { label: '40 MB', path: 'png/40MB.png' },
        { label: '80 MB', path: 'png/80MB.png' }
    ];

    var grid = document.getElementById('grid');
    var summary = document.getElementById('summary');
    var cards = IMAGES.map(createCard);

    document.getElementById('load-all').addEventListener('click', function () {
        cards.forEach(function (card) {
            if (card.state !== 'loading') load(card);
        });
    });

    document.getElementById('clear-all').addEventListener('click', function () {
        cards.forEach(clearCard);
    });

    updateSummary();

    /* ---------- Card creation ---------- */

    function createCard(info) {
        var el = document.createElement('article');
        el.className = 'card';
        el.innerHTML =
            '<div class="frame">' +
                '<img alt="" hidden>' +
                '<span class="placeholder">Not loaded</span>' +
            '</div>' +
            '<div class="card-body">' +
                '<div class="card-title"><h2></h2><code></code></div>' +
                '<div class="bar" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="0"><span></span></div>' +
                '<p class="status">Ready to load.</p>' +
                '<button class="btn btn-outline card-btn" type="button">Load image</button>' +
            '</div>';

        el.querySelector('h2').textContent = info.label;
        el.querySelector('code').textContent = info.path;
        el.querySelector('img').alt = info.label + ' PNG';

        var card = {
            info: info,
            el: el,
            img: el.querySelector('img'),
            placeholder: el.querySelector('.placeholder'),
            bar: el.querySelector('.bar'),
            fill: el.querySelector('.bar span'),
            status: el.querySelector('.status'),
            btn: el.querySelector('.card-btn'),
            state: 'idle',
            ctrl: null,
            url: null
        };

        card.btn.addEventListener('click', function () {
            if (card.state === 'loading') {
                card.ctrl.abort();          // user pressed Cancel
            } else {
                load(card);
            }
        });

        grid.appendChild(el);
        return card;
    }

    /* ---------- Loading ---------- */

    async function load(card) {
        releaseImage(card);
        card.ctrl = new AbortController();
        setState(card, 'loading', 'Connecting...');
        setProgress(card, 0);

        var started = performance.now();

        try {
            var res = await fetch(card.info.path, { cache: 'no-store', signal: card.ctrl.signal });
            if (!res.ok) {
                throw new Error('the server answered HTTP ' + res.status);
            }

            var total = Number(res.headers.get('Content-Length')) || 0;
            var reader = res.body.getReader();
            var chunks = [];
            var received = 0;

            while (true) {
                var part = await reader.read();
                if (part.done) break;
                chunks.push(part.value);
                received += part.value.length;

                var seconds = Math.max(performance.now() - started, 1) / 1000;
                setProgress(card, total ? (received / total) * 100 : 0);
                card.status.textContent =
                    formatBytes(received) + (total ? ' of ' + formatBytes(total) : '') +
                    ' at ' + formatBytes(received / seconds) + '/s';
            }

            var blob = new Blob(chunks, { type: res.headers.get('Content-Type') || 'image/png' });
            card.url = URL.createObjectURL(blob);
            card.img.src = card.url;
            card.img.hidden = false;
            card.placeholder.hidden = true;

            var elapsed = Math.max(performance.now() - started, 1) / 1000;
            setProgress(card, 100);
            setState(card, 'done',
                'Loaded ' + formatBytes(received) + ' in ' + elapsed.toFixed(2) + ' s at ' +
                formatBytes(received / elapsed) + '/s');
        } catch (err) {
            if (err === 'clear') {
                return;                     // Clear already reset this card
            }
            if (err && err.name === 'AbortError') {
                setProgress(card, 0);
                setState(card, 'idle', 'Canceled.');
            } else {
                setState(card, 'error', "Couldn't load the image: " + (err && err.message ? err.message : err));
            }
        } finally {
            card.ctrl = null;
            updateSummary();
        }
    }

    function clearCard(card) {
        if (card.ctrl) {
            card.ctrl.abort('clear');
        }
        releaseImage(card);
        setProgress(card, 0);
        setState(card, 'idle', 'Ready to load.');
        updateSummary();
    }

    function releaseImage(card) {
        if (card.url) {
            URL.revokeObjectURL(card.url);  // free the downloaded image from memory
            card.url = null;
        }
        card.img.hidden = true;
        card.img.removeAttribute('src');
        card.placeholder.hidden = false;
    }

    /* ---------- UI helpers ---------- */

    function setState(card, state, message) {
        card.state = state;
        card.el.classList.toggle('is-loading', state === 'loading');
        card.el.classList.toggle('is-done', state === 'done');
        card.el.classList.toggle('is-error', state === 'error');
        card.status.textContent = message;
        card.btn.textContent =
            state === 'loading' ? 'Cancel' :
            state === 'idle' ? 'Load image' : 'Load again';
        updateSummary();
    }

    function setProgress(card, percent) {
        var p = Math.min(100, Math.max(0, percent));
        card.fill.style.width = p + '%';
        card.bar.setAttribute('aria-valuenow', Math.round(p));
    }

    function updateSummary() {
        var done = cards.filter(function (c) { return c.state === 'done'; }).length;
        summary.textContent = done + ' of ' + cards.length + ' loaded';
    }

    function formatBytes(bytes) {
        var units = ['B', 'KB', 'MB', 'GB'];
        var i = 0;
        while (bytes >= 1024 && i < units.length - 1) {
            bytes /= 1024;
            i++;
        }
        return bytes.toFixed(i === 0 ? 0 : 1) + ' ' + units[i];
    }
})();
