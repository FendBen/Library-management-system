//
//  LBLibraryStore.m
//
#import "LBLibraryStore.h"

static NSString *const kFileName = @"library.json";

@implementation LBLibraryStore {
    NSMutableArray<LBBook *> *_books;
    NSMutableArray<LBReader *> *_readers;
    NSMutableArray<LBRecord *> *_records;
    NSInteger _nextBookSeq;
    NSInteger _nextReaderSeq;
    NSInteger _nextRecordSeq;
}

+ (instancetype)sharedStore {
    static LBLibraryStore *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[LBLibraryStore alloc] init];
    });
    return instance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _books = [NSMutableArray array];
        _readers = [NSMutableArray array];
        _records = [NSMutableArray array];
        _nextBookSeq = 1;
        _nextReaderSeq = 1;
        _nextRecordSeq = 1;
    }
    return self;
}

- (NSMutableArray<LBBook *> *)books { return _books; }
- (NSMutableArray<LBReader *> *)readers { return _readers; }
- (NSMutableArray<LBRecord *> *)records { return _records; }

#pragma mark - 路径与加载/保存

- (NSString *)dataPath {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                                         NSUserDomainMask, YES);
    NSString *dir = paths.firstObject;
    return [dir stringByAppendingPathComponent:kFileName];
}

- (void)load {
    NSString *path = [self dataPath];
    NSData *data = [NSData dataWithContentsOfFile:path];
    if (!data) return;  // 全新开始

    NSError *error = nil;
    NSDictionary *root = [NSJSONSerialization JSONObjectWithData:data
                                                        options:0
                                                          error:&error];
    if (!root || ![root isKindOfClass:[NSDictionary class]]) return;

    for (NSDictionary *d in root[@"books"] ?: @[])
        [_books addObject:[[LBBook alloc] initWithDictionary:d]];
    for (NSDictionary *d in root[@"readers"] ?: @[])
        [_readers addObject:[[LBReader alloc] initWithDictionary:d]];
    for (NSDictionary *d in root[@"records"] ?: @[])
        [_records addObject:[[LBRecord alloc] initWithDictionary:d]];

    for (LBBook *b in _books) {
        _nextBookSeq = MAX(_nextBookSeq, [self numberSuffix:b.bookId] + 1);
    }
    for (LBReader *r in _readers) {
        _nextReaderSeq = MAX(_nextReaderSeq, [self numberSuffix:r.readerId] + 1);
    }
    for (LBRecord *r in _records) {
        _nextRecordSeq = MAX(_nextRecordSeq, [self numberSuffix:r.recordId] + 1);
    }
}

- (NSInteger)numberSuffix:(NSString *)identifier {
    NSMutableString *digits = [NSMutableString string];
    for (NSUInteger i = 0; i < identifier.length; i++) {
        unichar c = [identifier characterAtIndex:i];
        if (c >= '0' && c <= '9') [digits appendFormat:@"%C", c];
    }
    return digits.length ? digits.integerValue : 0;
}

- (BOOL)save {
    NSArray *bookArr = [self arrayOfDictionaries:_books
                                       converter:^NSDictionary *(LBBook *b) {
                                           return [b toDictionary];
                                       }];
    NSArray *readerArr = [self arrayOfDictionaries:_readers
                                        converter:^NSDictionary *(LBReader *r) {
                                            return [r toDictionary];
                                        }];
    NSArray *recordArr = [self arrayOfDictionaries:_records
                                         converter:^NSDictionary *(LBRecord *r) {
                                             return [r toDictionary];
                                         }];
    NSDictionary *root = @{
        @"books": bookArr,
        @"readers": readerArr,
        @"records": recordArr,
    };
    NSError *error = nil;
    NSData *data = [NSJSONSerialization dataWithJSONObject:root
                                                   options:NSJSONWritingPrettyPrinted
                                                     error:&error];
    if (!data) return NO;
    return [data writeToFile:[self dataPath] atomically:YES];
}

- (NSArray *)arrayOfDictionaries:(NSArray *)items
                       converter:(NSDictionary * (^)(id item))converter {
    NSMutableArray *out = [NSMutableArray arrayWithCapacity:items.count];
    for (id item in items) [out addObject:converter(item)];
    return out;
}

#pragma mark - ID 生成

- (NSString *)nextBookId {
    NSString *id = nil;
    do {
        id = [NSString stringWithFormat:@"B%03ld", (long)_nextBookSeq++];
    } while ([self bookById:id] != nil);
    return id;
}

- (NSString *)nextReaderId {
    NSString *id = nil;
    do {
        id = [NSString stringWithFormat:@"R%03ld", (long)_nextReaderSeq++];
    } while ([self readerById:id] != nil);
    return id;
}

- (NSString *)nextRecordId {
    NSString *id = nil;
    do {
        id = [NSString stringWithFormat:@"BR%03ld", (long)_nextRecordSeq++];
        BOOL collide = NO;
        for (LBRecord *r in _records)
            if ([r.recordId isEqualToString:id]) { collide = YES; break; }
        if (!collide) return id;
    } while (YES);
}

#pragma mark - 图书

- (LBBook *)addBookTitle:(NSString *)title author:(NSString *)author
                    isbn:(NSString *)isbn copies:(NSInteger)copies
                   error:(NSString *_Nullable *_Nullable)err {
    if (err) *err = nil;
    NSString *t = [self trim:title];
    NSString *a = [self trim:author];
    NSString *i = [self trim:isbn];
    if (t.length == 0) { if (err) *err = @"书名不能为空"; return nil; }
    if (a.length == 0) { if (err) *err = @"作者不能为空"; return nil; }
    if (copies < 1) { if (err) *err = @"册数必须大于等于 1"; return nil; }
    if (i.length > 0) {
        for (LBBook *b in _books)
            if ([b.isbn isEqualToString:i]) {
                if (err) *err = [NSString stringWithFormat:@"ISBN 已存在：%@", i];
                return nil;
            }
    }
    LBBook *book = [[LBBook alloc] init];
    book.bookId = [self nextBookId];
    book.title = t;
    book.author = a;
    book.isbn = i;
    book.totalCopies = copies;
    book.availableCopies = copies;
    [_books addObject:book];
    [self save];
    return book;
}

- (BOOL)removeBook:(NSString *)bookId error:(NSString *_Nullable *_Nullable)err {
    if (err) *err = nil;
    LBBook *book = [self bookById:bookId];
    if (!book) { if (err) *err = @"图书不存在"; return NO; }
    if (book.availableCopies < book.totalCopies) {
        if (err) *err = @"该书还有未归还的借出记录，无法删除";
        return NO;
    }
    [_books removeObject:book];
    [self save];
    return YES;
}

- (BOOL)updateBook:(NSString *)bookId title:(NSString *)title author:(NSString *)author
              isbn:(NSString *)isbn copies:(NSInteger)copies
             error:(NSString *_Nullable *_Nullable)err {
    if (err) *err = nil;
    LBBook *book = [self bookById:bookId];
    if (!book) { if (err) *err = @"图书不存在"; return NO; }
    NSString *t = [self trim:title];
    NSString *a = [self trim:author];
    NSString *i = [self trim:isbn];
    if (t.length == 0) { if (err) *err = @"书名不能为空"; return NO; }
    if (a.length == 0) { if (err) *err = @"作者不能为空"; return NO; }
    if (copies < 1) { if (err) *err = @"册数必须大于等于 1"; return NO; }
    if (i.length > 0) {
        for (LBBook *b in _books)
            if (![b.bookId isEqualToString:bookId] && [b.isbn isEqualToString:i]) {
                if (err) *err = [NSString stringWithFormat:@"ISBN 已存在：%@", i];
                return NO;
            }
    }
    NSInteger borrowed = book.totalCopies - book.availableCopies;
    if (copies < borrowed) {
        if (err) *err = [NSString stringWithFormat:@"新册数不能少于当前借出册数（%ld）", (long)borrowed];
        return NO;
    }
    book.title = t;
    book.author = a;
    book.isbn = i;
    book.totalCopies = copies;
    book.availableCopies = copies - borrowed;
    [self save];
    return YES;
}

- (NSArray<LBBook *> *)searchBooks:(NSString *)keyword {
    NSString *kw = [self trim:keyword];
    if (kw.length == 0) return [_books copy];
    NSMutableArray *out = [NSMutableArray array];
    for (LBBook *b in _books) {
        if ([b.title localizedCaseInsensitiveContainsString:kw] ||
            [b.author localizedCaseInsensitiveContainsString:kw] ||
            [b.isbn localizedCaseInsensitiveContainsString:kw] ||
            [b.bookId localizedCaseInsensitiveContainsString:kw]) {
            [out addObject:b];
        }
    }
    return out;
}

- (LBBook *)bookById:(NSString *)bookId {
    for (LBBook *b in _books)
        if ([b.bookId isEqualToString:bookId]) return b;
    return nil;
}

#pragma mark - 读者

- (LBReader *)addReaderName:(NSString *)name phone:(NSString *)phone
                      error:(NSString *_Nullable *_Nullable)err {
    if (err) *err = nil;
    NSString *n = [self trim:name];
    if (n.length == 0) { if (err) *err = @"读者姓名不能为空"; return nil; }
    LBReader *reader = [[LBReader alloc] init];
    reader.readerId = [self nextReaderId];
    reader.name = n;
    reader.phone = [self trim:phone];
    [_readers addObject:reader];
    [self save];
    return reader;
}

- (BOOL)removeReader:(NSString *)readerId error:(NSString *_Nullable *_Nullable)err {
    if (err) *err = nil;
    LBReader *reader = [self readerById:readerId];
    if (!reader) { if (err) *err = @"读者不存在"; return NO; }
    for (LBRecord *r in _records)
        if (![r isReturned] && [r.readerId isEqualToString:readerId]) {
            if (err) *err = @"该读者还有未归还的图书，无法删除";
            return NO;
        }
    [_readers removeObject:reader];
    [self save];
    return YES;
}

- (LBReader *)readerById:(NSString *)readerId {
    for (LBReader *r in _readers)
        if ([r.readerId isEqualToString:readerId]) return r;
    return nil;
}

#pragma mark - 借还

- (BOOL)borrowBook:(NSString *)bookId byReader:(NSString *)readerId
              days:(NSInteger)days error:(NSString *_Nullable *_Nullable)err {
    if (err) *err = nil;
    if (days < 1) { if (err) *err = @"借期必须大于等于 1 天"; return NO; }
    if (![self readerById:readerId]) { if (err) *err = @"读者不存在"; return NO; }
    LBBook *book = [self bookById:bookId];
    if (!book) { if (err) *err = @"图书不存在"; return NO; }
    for (LBRecord *r in _records)
        if (![r isReturned] && [r.readerId isEqualToString:readerId] &&
            [r.bookId isEqualToString:bookId]) {
            if (err) *err = @"该读者已借阅此书且尚未归还";
            return NO;
        }
    if (book.availableCopies < 1) { if (err) *err = @"该书库存不足"; return NO; }

    LBRecord *rec = [[LBRecord alloc] init];
    rec.recordId = [self nextRecordId];
    rec.readerId = readerId;
    rec.bookId = bookId;
    rec.borrowDate = [LBLibraryStore todayString];
    rec.dueDate = [LBLibraryStore dateByAddingDays:days toDate:rec.borrowDate];
    [_records addObject:rec];
    book.availableCopies -= 1;
    [self save];
    return YES;
}

- (BOOL)returnBook:(NSString *)bookId byReader:(NSString *)readerId
             error:(NSString *_Nullable *_Nullable)err {
    if (err) *err = nil;
    LBRecord *target = nil;
    for (LBRecord *r in _records)
        if (![r isReturned] && [r.readerId isEqualToString:readerId] &&
            [r.bookId isEqualToString:bookId]) {
            target = r;
            break;
        }
    if (!target) {
        if (err) *err = @"未找到该读者对这本书的未归还记录";
        return NO;
    }
    target.returnDate = [LBLibraryStore todayString];
    LBBook *book = [self bookById:bookId];
    if (book) book.availableCopies += 1;
    [self save];
    return YES;
}

- (NSArray<LBRecord *> *)activeRecords {
    NSMutableArray *out = [NSMutableArray array];
    for (LBRecord *r in _records)
        if (![r isReturned]) [out addObject:r];
    return out;
}

- (BOOL)recordIsOverdue:(LBRecord *)record {
    if ([record isReturned]) return NO;
    return [LBLibraryStore date:record.dueDate earlierThan:[LBLibraryStore todayString]];
}

#pragma mark - 统计

- (NSDictionary *)stats {
    NSInteger titles = _books.count;
    NSInteger total = 0, available = 0, active = 0, overdue = 0;
    for (LBBook *b in _books) {
        total += b.totalCopies;
        available += b.availableCopies;
    }
    for (LBRecord *r in _records) {
        if (![r isReturned]) {
            active++;
            if ([self recordIsOverdue:r]) overdue++;
        }
    }
    return @{
        @"titles": @(titles),
        @"total": @(total),
        @"available": @(available),
        @"readers": @(_readers.count),
        @"active": @(active),
        @"overdue": @(overdue),
    };
}

#pragma mark - 日期

+ (NSDateFormatter *)dateFormatter {
    static NSDateFormatter *fmt = nil;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        fmt = [[NSDateFormatter alloc] init];
        fmt.locale = [NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"];
        fmt.dateFormat = @"yyyy-MM-dd";
    });
    return fmt;
}

+ (NSString *)todayString {
    return [[self dateFormatter] stringFromDate:[NSDate date]];
}

+ (NSString *)dateByAddingDays:(NSInteger)days toDate:(NSString *)date {
    NSDate *d = [[self dateFormatter] dateFromString:date];
    if (!d) d = [NSDate date];
    NSCalendar *cal = [NSCalendar currentCalendar];
    NSDate *newDate = [cal dateByAddingUnit:NSCalendarUnitDay value:days toDate:d options:0];
    return [[self dateFormatter] stringFromDate:newDate];
}

+ (BOOL)date:(NSString *)a earlierThan:(NSString *)b {
    NSDate *da = [[self dateFormatter] dateFromString:a];
    NSDate *db = [[self dateFormatter] dateFromString:b];
    if (!da || !db) return NO;
    return [da compare:db] == NSOrderedAscending;
}

#pragma mark - 工具

- (NSString *)trim:(NSString *)s {
    return [s stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
}

@end
