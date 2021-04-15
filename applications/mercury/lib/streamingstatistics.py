import math


class StreamingStatistics(object):
    def __init__(self):
        super(StreamingStatistics, self).__init__()
        self.n = 0
        self.mu = 0
        self.S = 0
        self.max = -math.inf
        self.min = math.inf

    def update(self, sample):
        # Equations from Wikipedia (https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm)
        #
        # Order of operations is important here -- cannot update S until we have updated mu
        prev_mu = self.mu
        self.n += 1
        self.mu += float(sample - self.mu) / self.n
        self.S += float(sample - prev_mu) * float(sample - self.mu)

        # Min / max are, comparatively easy (thankfully)
        self.max = max(self.max, sample)
        self.min = min(self.min, sample)

    def reset(self):
        self.n = 0
        self.mu = 0
        self.S = 0
        self.max = -math.inf
        self.min = math.inf

    def mean(self):
        return self.mu

    def standard_deviation(self):
        return math.sqrt(self.variance())

    def variance(self):
        if self.n >= 2:
            return self.S / (self.n - 1)
        else:
            return 0

    def maximum(self):
        return self.max

    def minimum(self):
        return self.min

    def count(self):
        return self.n

    def __repr__(self):
        return 'StreamingStatistics(MU={}, S={}, N={}, MAX={}, MIN={})'.format(self.mu, self.S, self.n, self.max,
                                                                               self.min)

    def __str__(self):
        msg = 'Mean = [{:12.06f}], standard deviation = [{:12.06f}], count = [{:d}], minimum = [{:12.06f}], maximum = [{:12.06f}]'
        return msg.format(self.mu, self.standard_deviation(), self.n, self.min, self.max)
