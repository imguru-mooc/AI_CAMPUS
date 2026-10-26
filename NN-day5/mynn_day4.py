# mynn.py — Neural Network 과정 미니 프레임워크 (Day 3 계층 + Day 4 학습 기법)
# Day 5 에서 Convolution · Pooling 계층을 이 파일에 추가합니다.
import numpy as np


# ───────────── 함수 ─────────────
def softmax(A):
    A = A - A.max(axis=-1, keepdims=True)
    expA = np.exp(A)
    return expA / expA.sum(axis=-1, keepdims=True)


def cross_entropy_error(y, t):
    if y.ndim == 1:
        y, t = y.reshape(1, -1), np.reshape(t, (1, -1))
    t = np.asarray(t)
    if t.size == y.size:
        t = t.argmax(axis=1)
    N = y.shape[0]
    return -np.sum(np.log(y[np.arange(N), t] + 1e-12)) / N


def numerical_gradient(f, x, h=1e-4):
    grad = np.zeros_like(x)
    it = np.nditer(x, flags=["multi_index"])
    while not it.finished:
        idx = it.multi_index
        orig = x[idx]
        x[idx] = orig + h; fxh1 = f(x)
        x[idx] = orig - h; fxh2 = f(x)
        grad[idx] = (fxh1 - fxh2) / (2 * h)
        x[idx] = orig
        it.iternext()
    return grad


def rel_error(a, b):
    return np.linalg.norm(a - b) / (np.linalg.norm(a) + np.linalg.norm(b) + 1e-12)


def init_std(weight_init, n_in):
    """'he' → √(2/n_in), 'xavier' → √(1/n_in), 숫자 → 그대로"""
    if weight_init == "he":
        return np.sqrt(2.0 / n_in)
    if weight_init == "xavier":
        return np.sqrt(1.0 / n_in)
    return float(weight_init)


# ───────────── 계층 ─────────────
# 모든 계층: params (이름 → 배열), grads (같은 이름 → 기울기), forward(x, train_flg), backward(dout)
class Layer:
    def __init__(self):
        self.params, self.grads = {}, {}
        self.buffers = {}                  # 학습되지 않지만 저장해야 하는 값 (BatchNorm 의 running 평균 · 분산)

    def forward(self, x, train_flg=True):
        raise NotImplementedError

    def backward(self, dout):
        raise NotImplementedError


class Affine(Layer):
    def __init__(self, input_size, output_size, weight_init="he", seed=0):
        super().__init__()
        rng = np.random.default_rng(seed)
        self.params["W"] = init_std(weight_init, input_size) * rng.standard_normal((input_size, output_size))
        self.params["b"] = np.zeros(output_size)

    def forward(self, x, train_flg=True):
        self.x_shape = x.shape                    # Day 5: (N, C, H, W) 입력도 펼쳐서 받는다
        self.x = x.reshape(x.shape[0], -1)
        return self.x @ self.params["W"] + self.params["b"]

    def backward(self, dout):
        self.grads["W"] = self.x.T @ dout
        self.grads["b"] = dout.sum(axis=0)
        return (dout @ self.params["W"].T).reshape(self.x_shape)


class Relu(Layer):
    def forward(self, x, train_flg=True):
        self.mask = (x <= 0)
        out = x.copy()
        out[self.mask] = 0
        return out

    def backward(self, dout):
        dx = dout.copy()
        dx[self.mask] = 0
        return dx


class Sigmoid(Layer):
    def forward(self, x, train_flg=True):
        self.out = 1 / (1 + np.exp(-x))
        return self.out

    def backward(self, dout):
        return dout * self.out * (1.0 - self.out)


class BatchNorm(Layer):
    def __init__(self, size, momentum=0.9, eps=1e-7):
        super().__init__()
        self.params["gamma"] = np.ones(size)
        self.params["beta"] = np.zeros(size)
        self.momentum, self.eps = momentum, eps
        self.buffers["running_mean"] = np.zeros(size)
        self.buffers["running_var"] = np.ones(size)

    def forward(self, x, train_flg=True):
        if train_flg:
            mu = x.mean(axis=0)
            self.xc = x - mu
            var = np.mean(self.xc ** 2, axis=0)
            self.std = np.sqrt(var + self.eps)
            self.xn = self.xc / self.std
            rm, rv = self.buffers["running_mean"], self.buffers["running_var"]
            rm[...] = self.momentum * rm + (1 - self.momentum) * mu        # 제자리 갱신
            rv[...] = self.momentum * rv + (1 - self.momentum) * var
        else:
            self.xn = (x - self.buffers["running_mean"]) / np.sqrt(self.buffers["running_var"] + self.eps)
        return self.params["gamma"] * self.xn + self.params["beta"]

    def backward(self, dout):
        N = dout.shape[0]
        self.grads["beta"] = dout.sum(axis=0)
        self.grads["gamma"] = np.sum(self.xn * dout, axis=0)
        dxn = self.params["gamma"] * dout
        return (N * dxn - dxn.sum(axis=0) - self.xn * np.sum(dxn * self.xn, axis=0)) / (N * self.std)


class Dropout(Layer):
    def __init__(self, ratio=0.5, seed=0):
        super().__init__()
        self.ratio = ratio
        self.rng = np.random.default_rng(seed)

    def forward(self, x, train_flg=True):
        if train_flg:
            self.mask = self.rng.random(x.shape) > self.ratio
            return x * self.mask
        return x * (1.0 - self.ratio)

    def backward(self, dout):
        return dout * self.mask


class SoftmaxWithLoss:
    def forward(self, x, t):
        self.t, self.y = t, softmax(x)
        self.loss = cross_entropy_error(self.y, t)
        return self.loss

    def backward(self, dout=1):
        N = self.y.shape[0]
        if self.t.size == self.y.size:
            return (self.y - self.t) / N * dout
        dx = self.y.copy()
        dx[np.arange(N), self.t] -= 1
        return dx / N * dout


# ───────────── 옵티마이저 ─────────────
class SGD:
    def __init__(self, lr=0.01):
        self.lr = lr

    def update(self, params, grads):
        for k in params:
            params[k] -= self.lr * grads[k]


class Momentum:
    def __init__(self, lr=0.01, momentum=0.9):
        self.lr, self.momentum, self.v = lr, momentum, None

    def update(self, params, grads):
        if self.v is None:
            self.v = {k: np.zeros_like(p) for k, p in params.items()}
        for k in params:
            self.v[k] = self.momentum * self.v[k] - self.lr * grads[k]
            params[k] += self.v[k]


class AdaGrad:
    def __init__(self, lr=0.01):
        self.lr, self.h = lr, None

    def update(self, params, grads):
        if self.h is None:
            self.h = {k: np.zeros_like(p) for k, p in params.items()}
        for k in params:
            self.h[k] += grads[k] ** 2
            params[k] -= self.lr * grads[k] / (np.sqrt(self.h[k]) + 1e-7)


class RMSProp:
    def __init__(self, lr=0.01, decay=0.99):
        self.lr, self.decay, self.h = lr, decay, None

    def update(self, params, grads):
        if self.h is None:
            self.h = {k: np.zeros_like(p) for k, p in params.items()}
        for k in params:
            self.h[k] = self.decay * self.h[k] + (1 - self.decay) * grads[k] ** 2
            params[k] -= self.lr * grads[k] / (np.sqrt(self.h[k]) + 1e-7)


class Adam:
    def __init__(self, lr=0.001, beta1=0.9, beta2=0.999):
        self.lr, self.beta1, self.beta2 = lr, beta1, beta2
        self.m, self.v, self.t = None, None, 0

    def update(self, params, grads):
        if self.m is None:
            self.m = {k: np.zeros_like(p) for k, p in params.items()}
            self.v = {k: np.zeros_like(p) for k, p in params.items()}
        self.t += 1
        for k in params:
            self.m[k] = self.beta1 * self.m[k] + (1 - self.beta1) * grads[k]
            self.v[k] = self.beta2 * self.v[k] + (1 - self.beta2) * grads[k] ** 2
            m_hat = self.m[k] / (1 - self.beta1 ** self.t)
            v_hat = self.v[k] / (1 - self.beta2 ** self.t)
            params[k] -= self.lr * m_hat / (np.sqrt(v_hat) + 1e-7)


# ───────────── 모델 · 학습 ─────────────
class Sequential:
    def __init__(self, layers, weight_decay_lambda=0.0):
        self.layers = layers
        self.loss_layer = SoftmaxWithLoss()
        self.weight_decay_lambda = weight_decay_lambda

    def params(self):
        """{'0.W': 배열, '0.b': 배열, '1.gamma': …} — 계층의 배열을 그대로(같은 객체) 모은다"""
        return {f"{i}.{name}": p for i, layer in enumerate(self.layers) for name, p in layer.params.items()}

    def grads(self):
        g = {}
        for i, layer in enumerate(self.layers):
            for name, d in layer.grads.items():
                if name == "W" and self.weight_decay_lambda:
                    d = d + self.weight_decay_lambda * layer.params["W"]     # weight decay 기울기 λW
                g[f"{i}.{name}"] = d
        return g

    def predict(self, x, train_flg=False):
        for layer in self.layers:
            x = layer.forward(x, train_flg)
        return x

    def loss(self, x, t, train_flg=False):
        L = self.loss_layer.forward(self.predict(x, train_flg), t)
        if self.weight_decay_lambda:
            L += 0.5 * self.weight_decay_lambda * sum(np.sum(l.params["W"] ** 2) for l in self.layers if "W" in l.params)
        return L

    def accuracy(self, x, t, batch_size=1000):
        correct = 0
        for i in range(0, len(x), batch_size):
            correct += np.sum(self.predict(x[i:i + batch_size]).argmax(axis=1) == t[i:i + batch_size])
        return correct / len(x)

    def gradient(self, x, t):
        self.loss(x, t, train_flg=True)
        dout = self.loss_layer.backward(1)
        for layer in reversed(self.layers):
            dout = layer.backward(dout)
        return self.grads()

    def count_params(self):
        return sum(p.size for p in self.params().values())

    def state(self):
        """저장용 — params + buffers (BatchNorm running 평균 · 분산)"""
        s = dict(self.params())
        s.update({f"{i}.{name}": b for i, layer in enumerate(self.layers) for name, b in layer.buffers.items()})
        return s


class Trainer:
    def __init__(self, model, optimizer, batch_size=100, seed=0):
        self.model, self.optimizer, self.batch_size = model, optimizer, batch_size
        self.rng = np.random.default_rng(seed)
        self.history = {"loss": [], "train_acc": [], "val_acc": []}

    def fit(self, X, t, epochs=1, X_val=None, t_val=None, verbose=True):
        N = len(X)
        iters_per_epoch = max(N // self.batch_size, 1)
        params = self.model.params()
        for epoch in range(1, epochs + 1):
            idx = self.rng.permutation(N)
            for i in range(iters_per_epoch):
                batch = idx[i * self.batch_size:(i + 1) * self.batch_size]
                grads = self.model.gradient(X[batch], t[batch])
                self.optimizer.update(params, grads)
                self.history["loss"].append(float(self.model.loss_layer.loss))
            self.history["train_acc"].append(self.model.accuracy(X, t))
            if X_val is not None:
                self.history["val_acc"].append(self.model.accuracy(X_val, t_val))
            if verbose:
                val = f"  val {self.history['val_acc'][-1]:.4f}" if X_val is not None else ""
                print(f"epoch {epoch:3d}  loss {np.mean(self.history['loss'][-iters_per_epoch:]):.4f}  "
                      f"train {self.history['train_acc'][-1]:.4f}{val}")
        return self.history


def save_params(model, path):
    np.savez(path, **model.state())


def load_params(model, path):
    data = np.load(path)
    for k, p in model.state().items():
        p[...] = data[k]                  # 제자리 복사 — 계층 · 옵티마이저와의 연결 유지
