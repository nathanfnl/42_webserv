<?php
/**
 * PHP CGI Environment Inspector, Cookie Diagnostic & File Manager
 * 100% self-contained. No external dependencies.
 */

// 1. File Streaming & Download Pipeline (Must run before any HTML output)
if (isset($_GET['download'])) {
    $file_to_download = trim($_GET['download']);
    $clean_download_name = basename($file_to_download);
    $target_download_path = __DIR__ . '/' . $clean_download_name;

    if (!empty($clean_download_name) && file_exists($target_download_path) && is_file($target_download_path)) {
        if (ob_get_level()) { ob_end_clean(); }
        
        header('Content-Description: File Transfer');
        header('Content-Type: application/octet-stream');
        header('Content-Disposition: attachment; filename="' . $clean_download_name . '"');
        header('Expires: 0');
        header('Cache-Control: must-revalidate');
        header('Pragma: public');
        header('Content-Length: ' . filesize($target_download_path));
        
        readfile($target_download_path);
        exit;
    }
}

// 2. Dynamic Cookie Management (Creation & Deletion Pipeline)
$status_message = '';
$status_success = null;

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['action'])) {
    // Custom Cookie Creation
    if ($_POST['action'] === 'create_cookie') {
        $c_name = preg_replace('/[^a-zA-Pl0-9_]/', '', trim($_POST['cookie_name'] ?? ''));
        $c_value = trim($_POST['cookie_value'] ?? '');
        $c_expiry = intval($_POST['cookie_expiry'] ?? 3600);

        if (empty($c_name)) {
            $status_success = false;
            $status_message = 'Cookie name cannot be empty and must be alphanumeric/underscores.';
        } else {
            setcookie($c_name, $c_value, time() + $c_expiry, "/");
            header("Location: " . $_SERVER['PHP_SELF'] . "?status=cookie_created");
            exit;
        }
    }
}

// Handle Direct Cookie Drop Requests (via GET flag for easy integration)
if (isset($_GET['delete_cookie'])) {
    $cookie_to_drop = trim($_GET['delete_cookie']);
    if (isset($_COOKIE[$cookie_to_drop])) {
        setcookie($cookie_to_drop, '', time() - 3600, "/");
        header("Location: " . $_SERVER['PHP_SELF'] . "?status=cookie_deleted");
        exit;
    }
}

// Default Fallback Diagnostic Cookie
$default_cookie_name = 'cgi_test_cookie';
if (empty($_COOKIE)) {
    setcookie($default_cookie_name, 'PHP-CGI-Working-' . date('H:i:s'), time() + 3600, "/");
}

// 3. Handle File Creation & Multipart Uploads
$post_received = !empty($_POST) || !empty($_FILES);
$test_input = $_POST['test_input'] ?? '';

if ($post_received && isset($_POST['action'])) {
    // Action A: Create File Manually
    if ($_POST['action'] === 'create_file') {
        $new_filename = trim($_POST['new_filename'] ?? '');
        $new_filecontent = $_POST['new_filecontent'] ?? '';

        if (empty($new_filename)) {
            $status_success = false;
            $status_message = 'Filename cannot be blank.';
        } else {
            $clean_new_filename = basename($new_filename);
            $target_write_path = __DIR__ . '/' . $clean_new_filename;

            if ($clean_new_filename === basename(__FILE__)) {
                $status_success = false;
                $status_message = 'Cannot overwrite the inspector script itself.';
            } else {
                if (@file_put_contents($target_write_path, $new_filecontent) !== false) {
                    header("Location: " . $_SERVER['PHP_SELF'] . "?status=created");
                    exit;
                } else {
                    $status_success = false;
                    $status_message = 'Write permission error: Failed to save file to disk.';
                }
            }
        }
    }
    
    // Action B: Handle Uploaded File
    if ($_POST['action'] === 'upload_file' && isset($_FILES['uploaded_asset'])) {
        $upload = $_FILES['uploaded_asset'];
        
        if ($upload['error'] !== UPLOAD_ERR_OK) {
            $status_success = false;
            $status_message = 'Upload failed with system error code: ' . $upload['error'];
        } else {
            $clean_upload_name = basename($upload['name']);
            $target_upload_path = __DIR__ . '/' . $clean_upload_name;
            
            if ($clean_upload_name === basename(__FILE__)) {
                $status_success = false;
                $status_message = 'Security exception: Overwriting the diagnostic engine is forbidden.';
            } else {
                if (@move_uploaded_file($upload['tmp_name'], $target_upload_path)) {
                    header("Location: " . $_SERVER['PHP_SELF'] . "?status=uploaded");
                    exit;
                } else {
                    $status_success = false;
                    $status_message = 'Workspace error: Failed to shift uploaded asset into directory.';
                }
            }
        }
    }
}

// Catch redirection statuses
if (isset($_GET['status'])) {
    $status_success = true;
    if ($_GET['status'] === 'created') $status_message = "File dropped onto disk successfully!";
    if ($_GET['status'] === 'uploaded') $status_message = "Asset received and written to workspace context!";
    if ($_GET['status'] === 'cookie_created') $status_message = "Custom cookie jar updated and dropped successfully!";
    if ($_GET['status'] === 'cookie_deleted') $status_message = "Target cookie has been cleared from environmental scope.";
}

// 4. Handle incoming HTTP DELETE requests (File Management API)
if ($_SERVER['REQUEST_METHOD'] === 'DELETE') {
    header('Content-Type: application/json');
    parse_str($_SERVER['QUERY_STRING'], $query_vars);
    $file_to_delete = $query_vars['file'] ?? '';

    if (!empty($file_to_delete)) {
        $clean_filename = basename($file_to_delete);
        $target_path = __DIR__ . '/' . $clean_filename;

        if ($clean_filename === basename(__FILE__)) {
            http_response_code(403);
            echo json_encode(['success' => false, 'message' => 'For safety reasons, you cannot delete the inspector script itself.']);
            exit;
        }

        if (file_exists($target_path) && is_file($target_path)) {
            if (@unlink($target_path)) {
                echo json_encode(['success' => true, 'message' => "File '{$clean_filename}' dropped successfully."]);
                exit;
            } else {
                http_response_code(500);
                echo json_encode(['success' => false, 'message' => 'Write permissions error: System failed to unlink file.']);
                exit;
            }
        }
    }
    
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Invalid file target or path target is a directory.']);
    exit;
}

// 5. Catch headers safely across odd CGI gateways
$headers = function_exists('getallheaders') ? getallheaders() : [];
if (empty($headers)) {
    foreach ($_SERVER as $name => $value) {
        if (substr($name, 0, 5) == 'HTTP_') {
            $headers[str_replace(' ', '-', ucwords(strtolower(str_replace('_', ' ', substr($name, 5)))))] = $value;
        }
    }
}

// 6. Critical Gateway Health Metrics
$cgi_checks = [
    'Gateway Interface' => $_SERVER['GATEWAY_INTERFACE'] ?? 'Direct Process Execution',
    'Server Software' => $_SERVER['SERVER_SOFTWARE'] ?? 'Unknown',
    'PHP Interface Mode' => php_sapi_name(),
    'HTTPS Protocol' => (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off') ? '🔒 Secure' : '🔓 Unsecure / Plain HTTP',
];

// 7. Read Local Storage Directory Space
$directory_files = [];
if ($handle = opendir(__DIR__)) {
    while (false !== ($entry = readdir($handle))) {
        if ($entry != "." && $entry != "..") {
            $full_path = __DIR__ . '/' . $entry;
            $directory_files[] = [
                'name' => $entry,
                'is_dir' => is_dir($full_path),
                'size' => is_file($full_path) ? filesize($full_path) : 0,
                'mtime' => filemtime($full_path)
            ];
        }
    }
    closedir($handle);
    
    usort($directory_files, function($a, $b) {
        if ($a['is_dir'] != $b['is_dir']) {
            return $b['is_dir'] <=> $a['is_dir'];
        }
        return strcasecmp($a['name'], $b['name']);
    });
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Integrated CGI Gateway & Cookie Inspector</title>
    <style>
        :root {
            --bg: #090d16;
            --surface: #131926;
            --surface-hover: #1e2638;
            --border: #26334d;
            --text-main: #f1f5f9;
            --text-muted: #94a3b8;
            --accent: #38bdf8;
            --accent-purple: #a78bfa;
            --code-bg: #0b0f19;
            --status-color: #38bdf8;
            --status-bg: rgba(56, 189, 248, 0.1);
            --status-border: #38bdf8;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            background-color: var(--bg);
            color: var(--text-main);
            margin: 0;
            padding: 40px 20px;
            display: flex;
            justify-content: center;
        }

        .wrapper { width: 100%; max-width: 1000px; }

        header {
            margin-bottom: 32px;
            border-bottom: 1px solid var(--border);
            padding-bottom: 24px;
            display: flex;
            justify-content: space-between;
            align-items: flex-end;
            flex-wrap: wrap;
            gap: 16px;
        }

        h1 {
            font-size: 28px;
            font-weight: 800;
            margin: 0 0 6px 0;
            letter-spacing: -0.02em;
            background: linear-gradient(to right, #38bdf8, #a78bfa);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        header p { margin: 0; color: var(--text-muted); font-size: 14px; }

        .badge-sapi {
            background-color: var(--border);
            border: 1px solid var(--accent);
            color: var(--accent);
            padding: 4px 12px;
            border-radius: 6px;
            font-family: monospace;
            font-size: 13px;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
            gap: 24px;
            margin-bottom: 24px;
        }

        .card {
            background: var(--surface);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 24px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.25);
        }

        .card h2 {
            font-size: 16px;
            font-weight: 600;
            margin: 0 0 16px 0;
            display: flex;
            align-items: center;
            gap: 8px;
        }

        .alert {
            padding: 12px;
            border-radius: 6px;
            font-size: 13px;
            margin-bottom: 16px;
        }
        .alert-success { background: rgba(52, 211, 153, 0.1); border: 1px solid #34d399; color: #34d399; }
        .alert-error { background: rgba(239, 68, 68, 0.1); border: 1px solid #ef4444; color: #f87171; }

        .kv-table { width: 100%; border-collapse: collapse; font-size: 13.5px; }
        .kv-table td { padding: 10px 0; border-bottom: 1px solid rgba(38, 51, 77, 0.5); vertical-align: middle; }
        .kv-table tr:last-child td { border-bottom: none; }
        
        .key { color: var(--text-muted); font-weight: 500; word-break: break-all; padding-right: 8px; }
        .value { font-family: monospace; color: var(--text-main); word-break: break-all; text-align: right; }
        .value-highlight { color: #34d399; }

        .dump-box {
            background: var(--code-bg);
            border: 1px solid var(--border);
            border-radius: 8px;
            padding: 14px;
            font-family: monospace;
            font-size: 12.5px;
            overflow-x: auto;
            max-height: 200px;
            margin: 0;
        }

        .form-group { display: flex; gap: 12px; margin-top: 12px; }
        .vertical-form { display: flex; flex-direction: column; gap: 12px; }
        
        input[type="text"], input[type="file"], textarea, select {
            background: var(--code-bg); border: 1px solid var(--border);
            border-radius: 6px; padding: 10px 14px; color: var(--text-main); font-size: 14px; outline: none;
            font-family: inherit;
        }
        select { cursor: pointer; }
        input[type="file"] { padding: 8px; font-size: 13px; font-family: sans-serif; cursor: pointer; }
        textarea { font-family: monospace; font-size: 13px; resize: vertical; min-height: 100px; }
        input[type="text"]:focus, textarea:focus, select:focus { border-color: var(--accent); }

        .btn {
            display: inline-flex; justify-content: center; align-items: center;
            padding: 10px 18px; border-radius: 6px; font-size: 13.5px; font-weight: 600;
            text-decoration: none; cursor: pointer; border: none; transition: opacity 0.15s;
        }
        .btn:hover { opacity: 0.9; }
        .btn-primary { background: var(--accent); color: var(--bg); }
        .btn-secondary { background: rgba(255,255,255,0.05); color: var(--text-main); border: 1px solid var(--border); }
        .btn-success { background: rgba(52, 211, 153, 0.2); color: #34d399; border: 1px solid #34d399; }
        .btn-danger { background: rgba(239, 68, 68, 0.2); color: #f87171; border: 1px solid #ef4444; }
        .btn-xs { padding: 4px 8px; font-size: 11px; border-radius: 4px; margin-left: 4px; }

        .action-bar { display: flex; gap: 12px; margin-top: 16px; }
        .file-table th { text-align: left; color: var(--text-muted); font-size: 12px; padding-bottom: 8px; font-weight: 600; }
        .file-table td { vertical-align: middle; }
        .file-icon { margin-right: 6px; }
    </style>
</head>
<body>

<div class="wrapper">
    <header>
        <div>
            <h1>Gateway Inspector Pro</h1>
            <p>Unified Cookie State, Request Pipeline, & SAPI Tracker</p>
        </div>
        <div class="badge-sapi">Engine: <?php echo php_sapi_name(); ?></div>
    </header>

    <!-- Unified Status Alert Messaging -->
    <?php if ($status_message): ?>
        <div class="alert <?php echo $status_success ? 'alert-success' : 'alert-error'; ?>" style="margin-bottom: 24px;">
            <?php echo htmlspecialchars($status_message); ?>
        </div>
    <?php endif; ?>

    <!-- Cookie Modification Module & Gateway Profile -->
    <div class="grid">
        <div class="card">
            <h2>🍪 Drop / Mint Custom Cookie</h2>
            <form method="POST" action="?" class="vertical-form">
                <input type="hidden" name="action" value="create_cookie">
                
                <div style="display: flex; flex-direction: column; gap: 4px;">
                    <label style="font-size: 12px; color: var(--text-muted); font-weight: 600;">Cookie Name</label>
                    <input type="text" name="cookie_name" placeholder="e.g., user_session" required>
                </div>

                <div style="display: flex; flex-direction: column; gap: 4px;">
                    <label style="font-size: 12px; color: var(--text-muted); font-weight: 600;">Payload Data Value</label>
                    <input type="text" name="cookie_value" placeholder="e.g., token_val_9921" required>
                </div>

                <div style="display: flex; flex-direction: column; gap: 4px;">
                    <label style="font-size: 12px; color: var(--text-muted); font-weight: 600;">Lifespan Expiry</label>
                    <select name="cookie_expiry">
                        <option value="3600">1 Hour</option>
                        <option value="86400">1 Day</option>
                        <option value="604800">1 Week</option>
                        <option value="2112912000">67 Year</option>
                    </select>
                </div>

                <div style="text-align: right; margin-top: 4px;">
                    <button type="submit" class="btn btn-primary" style="width:100%;">🍪 Commit Cookie to Jar</button>
                </div>
            </form>
        </div>

        <div class="card" style="display: flex; flex-direction: column; justify-content: space-between;">
            <div>
                <h2>📋 Active Tracked Cookies</h2>
                <p style="color: var(--text-muted); font-size: 13px; margin: 0 0 12px 0;">Live variables available inside current client execution frame:</p>
                
                <div style="max-height: 210px; overflow-y: auto; border: 1px solid var(--border); border-radius: 6px; padding: 4px 12px; background: var(--code-bg);">
                    <table class="kv-table">
                        <?php if (empty($_COOKIE)): ?>
                            <tr><td colspan="2" style="color: var(--text-muted); font-style: italic; text-align: center; padding: 12px 0;">No context cookies active.</td></tr>
                        <?php else: ?>
                            <?php foreach ($_COOKIE as $name => $value): ?>
                                <tr>
                                    <td class="key" style="color: var(--accent-purple); font-family: monospace; font-size:12.5px;"><?php echo htmlspecialchars($name); ?></td>
                                    <td class="value" style="font-size:12.5px; max-width: 140px; text-overflow: ellipsis; overflow: hidden; white-space: nowrap;">
                                        <?php echo htmlspecialchars($value); ?>
                                    </td>
                                    <td style="text-align: right; width: 45px; padding-left: 8px;">
                                        <a href="?delete_cookie=<?php echo urlencode($name); ?>" class="btn btn-danger btn-xs" style="margin:0; padding: 2px 6px;">Nuke</a>
                                    </td>
                                </tr>
                            <?php endforeach; ?>
                        <?php endif; ?>
                    </table>
                </div>
            </div>
            <div class="action-bar" style="margin-top: 12px;">
                <a href="?" class="btn btn-secondary" style="width: 100%; text-align: center;">🔄 Refresh Environment State</a>
            </div>
        </div>
    </div>

    <!-- Dual File Creation & Multipart Upload Modules -->
    <div class="grid">
        <div class="card">
            <h2>🛠️ Create New Asset File</h2>
            <form method="POST" action="?" class="vertical-form">
                <input type="hidden" name="action" value="create_file">
                
                <div style="display: flex; flex-direction: column; gap: 4px;">
                    <label style="font-size: 12px; color: var(--text-muted); font-weight: 600;">Filename</label>
                    <input type="text" name="new_filename" placeholder="example.txt, debug.json..." required>
                </div>

                <div style="display: flex; flex-direction: column; gap: 4px;">
                    <label style="font-size: 12px; color: var(--text-muted); font-weight: 600;">File Contents</label>
                    <textarea name="new_filecontent" placeholder="Type or paste payload strings here..."></textarea>
                </div>

                <div style="text-align: right;">
                    <button type="submit" class="btn btn-primary">💾 Commit File</button>
                </div>
            </form>
        </div>

        <div class="card" style="display: flex; flex-direction: column; justify-content: space-between;">
            <div>
                <h2>📤 Stream & Upload Asset</h2>
                <p style="color: var(--text-muted); font-size: 13px; margin: 0 0 16px 0;">
                    Select a local system binary or text log format to drop straight onto your runtime workspace directory target.
                </p>
            </div>
            <form method="POST" action="?" enctype="multipart/form-data" class="vertical-form">
                <input type="hidden" name="action" value="upload_file">
                <div style="display: flex; flex-direction: column; gap: 8px;">
                    <label style="font-size: 12px; color: var(--text-muted); font-weight: 600;">Select Native Asset</label>
                    <input type="file" name="uploaded_asset" required>
                </div>
                <div style="text-align: right; margin-top: 12px;">
                    <button type="submit" class="btn btn-primary" style="width: 100%;">🚀 Execute Direct Stream Upload</button>
                </div>
            </form>
        </div>
    </div>

    <!-- Dynamic Directory & File System Explorer -->
    <div class="card" style="margin-bottom: 24px;">
        <h2>📂 Directory Workspace Storage Engine (`DELETE` & `GET` Pipelines)</h2>
        <p style="color: var(--text-muted); font-size: 13px; margin: 0 0 16px 0;">
            Current Directory Range: <code style="color: var(--accent-purple); font-size: 12.5px;"><?php echo htmlspecialchars(__DIR__); ?></code>
        </p>
        <table class="kv-table file-table">
            <thead>
                <tr>
                    <th style="text-align: left;">Target Asset Item</th>
                    <th style="text-align: right;">Size</th>
                    <th style="text-align: right;">Modified Time</th>
                    <th style="text-align: right;">Operations Pipeline</th>
                </tr>
            </thead>
            <tbody>
                <?php if (empty($directory_files)): ?>
                <tr>
                    <td colspan="4" style="color: var(--text-muted); font-style: italic; padding: 12px 0;">No sibling filesystem entries detected inside directory scope.</td>
                </tr>
                <?php else: ?>
                    <?php foreach ($directory_files as $file): ?>
                    <tr>
                        <td style="font-family: monospace; font-weight: 500;">
                            <?php if ($file['is_dir']): ?>
                                <span class="file-icon">📁</span><span style="color: var(--accent-purple);"><?php echo htmlspecialchars($file['name']); ?>/</span>
                            <?php else: ?>
                                <span class="file-icon">📄</span><span style="color: var(--text-main);"><?php echo htmlspecialchars($file['name']); ?></span>
                            <?php endif; ?>
                        </td>
                        <td class="value" style="font-size: 12.5px;">
                            <?php echo $file['is_dir'] ? '<span style="color:var(--text-muted);">[DIR]</span>' : number_format($file['size']) . ' B'; ?>
                        </td>
                        <td class="value" style="font-size: 12.5px; color: var(--text-muted);">
                            <?php echo date('Y-m-d H:i:s', $file['mtime']); ?>
                        </td>
                        <td style="text-align: right;">
                            <?php if (!$file['is_dir'] && $file['name'] !== basename(__FILE__)): ?>
                                <a href="?download=<?php echo urlencode($file['name']); ?>" class="btn btn-success btn-xs">HTTP GET (📥)</a>
                                <button onclick="triggerFileDelete('<?php echo htmlspecialchars(addslashes($file['name'])); ?>')" class="btn btn-danger btn-xs">HTTP DELETE</button>
                            <?php elseif ($file['name'] === basename(__FILE__)): ?>
                                <span style="font-size: 11px; color: var(--text-muted); padding: 4px; border: 1px dashed var(--border); border-radius:4px;">🔒 Lock System</span>
                            <?php else: ?>
                                <span style="font-size: 11px; color: var(--text-muted);">-</span>
                            <?php endif; ?>
                        </td>
                    </tr>
                    <?php endforeach; ?>
                <?php endif; ?>
            </tbody>
        </table>
    </div>

    <!-- Core Gateway Profile & POST Sandbox -->
    <div class="grid">
        <div class="card">
            <h2>⚙️ Core Gateway Profile</h2>
            <table class="kv-table">
                <?php foreach ($cgi_checks as $k => $v): ?>
                <tr>
                    <td class="key"><?php echo htmlspecialchars($k); ?></td>
                    <td class="value value-highlight"><?php echo htmlspecialchars($v); ?></td>
                </tr>
                <?php endforeach; ?>
                <tr>
                    <td class="key">Active Query Errors</td>
                    <td class="value" style="color: <?php echo isset($_GET['?yolo']) ? '#f87171' : '#34d399'; ?>;">
                        <?php echo isset($_GET['?yolo']) ? '⚠️ URL includes literal "?" in key!' : 'None Detected'; ?>
                    </td>
                </tr>
            </table>
        </div>

        <div class="card">
            <h2>🧪 POST Request Payload Sandbox</h2>
            <p style="color: var(--text-muted); font-size: 13px; margin: 0 0 12px 0;">Test how your gateway environment captures and holds form payloads.</p>
            <form method="POST" action="?" style="margin-top: 15px;">
                <div class="form-group">
                    <input type="text" name="test_input" placeholder="Type data payload string..." value="<?php echo htmlspecialchars($test_input); ?>" style="flex: 1;" required>
                    <button type="submit" class="btn btn-secondary">Send POST</button>
                </div>
            </form>
            <?php if ($post_received && (!isset($_POST['action']))): ?>
                <div style="color: #34d399; font-size: 12px; margin-top: 10px; font-weight: 500;">✓ Form payload safely processed by web gateway container.</div>
            <?php endif; ?>
        </div>
    </div>

    <!-- Scoped Memory Dumps -->
    <div class="grid">
        <div class="card">
            <h2>📥 URL Arguments ($_GET)</h2>
            <pre class="dump-box" style="color: #67e8f9;"><?php echo empty($_GET) ? '// Array empty. Append query string arguments to test.' : htmlspecialchars(print_r($_GET, true)); ?></pre>
        </div>
        <div class="card">
            <h2>📤 Active Form Data ($_POST)</h2>
            <pre class="dump-box" style="color: #fca5a5;"><?php echo empty($_POST) ? '// Array empty. Use the form payload box above.' : htmlspecialchars(print_r($_POST, true)); ?></pre>
        </div>
    </div>

    <!-- Incoming Network Request Headers -->
    <div class="card" style="margin-bottom: 24px;">
        <h2>🌐 Captured HTTP Request Headers</h2>
        <table class="kv-table">
            <?php foreach ($headers as $key => $val): ?>
            <tr>
                <td class="key" style="font-family: monospace; color: #f472b6;"><?php echo htmlspecialchars($key); ?></td>
                <td class="value"><?php echo htmlspecialchars($val); ?></td>
            </tr>
            <?php endforeach; ?>
        </table>
    </div>

    <!-- Full Diagnostics Configuration -->
    <div class="card">
        <h2>📂 Full Engine Scope Environment ($_SERVER)</h2>
        <pre class="dump-box" style="max-height: 350px; color: #cbd5e1;"><?php echo htmlspecialchars(print_r($_SERVER, true)); ?></pre>
    </div>
</div>

<script>
async function triggerFileDelete(filename) {
    if (!confirm(`Are you sure you want to execute an HTTP DELETE request to remove '${filename}'?`)) {
        return;
    }

    try {
        const response = await fetch(`?file=${encodeURIComponent(filename)}`, {
            method: 'DELETE'
        });

        const data = await response.json();
        
        if (response.ok && data.success) {
            alert(data.message || "File purged cleanly.");
            window.location.reload();
        } else {
            alert(`Error response [${response.status}]: ${data.message || 'Deletion failed.'}`);
        }
    } catch (err) {
        alert(`Pipeline failure executing request payload: ${err.message}`);
    }
}
</script>

</body>
</html>