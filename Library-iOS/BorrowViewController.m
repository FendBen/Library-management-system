//
//  BorrowViewController.m
//
#import "BorrowViewController.h"
#import "LBLibraryStore.h"
#import "LBBook.h"
#import "LBReader.h"
#import "LBRecord.h"

@interface BorrowViewController () <UITableViewDataSource, UITableViewDelegate,
                                    UITextFieldDelegate>
@property (nonatomic, strong) UITextField *readerField;
@property (nonatomic, strong) UITextField *bookField;
@property (nonatomic, strong) UITextField *daysField;
@property (nonatomic, strong) UIButton *borrowButton;
@property (nonatomic, strong) UILabel *summaryLabel;
@property (nonatomic, strong) UITableView *tableView;
@property (nonatomic, strong) NSArray<LBRecord *> *records;

@property (nonatomic, strong, nullable) LBReader *selectedReader;
@property (nonatomic, strong, nullable) LBBook *selectedBook;
@end

@implementation BorrowViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"借阅";
    self.view.backgroundColor = [UIColor systemBackgroundColor];

    UILabel *readerLbl = [self makeLabel:@"读者"];
    UILabel *bookLbl = [self makeLabel:@"图书"];
    UILabel *daysLbl = [self makeLabel:@"借期(天)"];

    self.readerField = [self makeField:@"选择读者" keyboard:UIKeyboardTypeDefault];
    self.bookField = [self makeField:@"选择图书" keyboard:UIKeyboardTypeDefault];
    self.daysField = [self makeField:@"14" keyboard:UIKeyboardTypeNumberPad];
    self.daysField.delegate = self;

    self.borrowButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [self.borrowButton setTitle:@"借 书" forState:UIControlStateNormal];
    self.borrowButton.titleLabel.font = [UIFont boldSystemFontOfSize:17];
    [self.borrowButton addTarget:self action:@selector(borrowTapped)
                forControlEvents:UIControlEventTouchUpInside];

    self.summaryLabel = [self makeLabel:@""];
    self.summaryLabel.font = [UIFont systemFontOfSize:13];
    self.summaryLabel.textColor = [UIColor secondaryLabelColor];
    self.summaryLabel.textAlignment = NSTextAlignmentCenter;

    self.tableView = [[UITableView alloc] initWithFrame:CGRectZero
                                                  style:UITableViewStylePlain];
    self.tableView.dataSource = self;
    self.tableView.delegate = self;
    [self.tableView registerClass:[UITableViewCell class]
           forCellReuseIdentifier:@"RecordCell"];
    self.tableView.translatesAutoresizingMaskIntoConstraints = NO;

    UIView *form = [[UIView alloc] init];
    form.translatesAutoresizingMaskIntoConstraints = NO;
    [form addSubview:readerLbl];
    [form addSubview:self.readerField];
    [form addSubview:bookLbl];
    [form addSubview:self.bookField];
    [form addSubview:daysLbl];
    [form addSubview:self.daysField];
    [form addSubview:self.borrowButton];

    [self.view addSubview:form];
    [self.view addSubview:self.summaryLabel];
    [self.view addSubview:self.tableView];

    NSDictionary *views = @{
        @"readerLbl": readerLbl, @"readerField": self.readerField,
        @"bookLbl": bookLbl, @"bookField": self.bookField,
        @"daysLbl": daysLbl, @"daysField": self.daysField,
        @"borrowButton": self.borrowButton,
    };
    for (UIView *v in views.allValues) {
        v.translatesAutoresizingMaskIntoConstraints = NO;
    }

    [NSLayoutConstraint activateConstraints:@[
        [form.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:8],
        [form.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:16],
        [form.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-16],

        [readerLbl.topAnchor constraintEqualToAnchor:form.topAnchor],
        [readerLbl.leadingAnchor constraintEqualToAnchor:form.leadingAnchor],
        [readerLbl.widthAnchor constraintEqualToConstant:72],
        [self.readerField.centerYAnchor constraintEqualToAnchor:readerLbl.centerYAnchor],
        [self.readerField.leadingAnchor constraintEqualToAnchor:readerLbl.trailingAnchor constant:8],
        [self.readerField.trailingAnchor constraintEqualToAnchor:form.trailingAnchor],
        [self.readerField.heightAnchor constraintEqualToConstant:34],

        [bookLbl.topAnchor constraintEqualToAnchor:readerLbl.bottomAnchor constant:10],
        [bookLbl.leadingAnchor constraintEqualToAnchor:form.leadingAnchor],
        [bookLbl.widthAnchor constraintEqualToConstant:72],
        [self.bookField.centerYAnchor constraintEqualToAnchor:bookLbl.centerYAnchor],
        [self.bookField.leadingAnchor constraintEqualToAnchor:bookLbl.trailingAnchor constant:8],
        [self.bookField.trailingAnchor constraintEqualToAnchor:form.trailingAnchor],
        [self.bookField.heightAnchor constraintEqualToConstant:34],

        [daysLbl.topAnchor constraintEqualToAnchor:bookLbl.bottomAnchor constant:10],
        [daysLbl.leadingAnchor constraintEqualToAnchor:form.leadingAnchor],
        [daysLbl.widthAnchor constraintEqualToConstant:72],
        [self.daysField.centerYAnchor constraintEqualToAnchor:daysLbl.centerYAnchor],
        [self.daysField.leadingAnchor constraintEqualToAnchor:daysLbl.trailingAnchor constant:8],
        [self.daysField.widthAnchor constraintEqualToConstant:80],
        [self.daysField.heightAnchor constraintEqualToConstant:34],

        [self.borrowButton.topAnchor constraintEqualToAnchor:daysLbl.bottomAnchor constant:12],
        [self.borrowButton.leadingAnchor constraintEqualToAnchor:form.leadingAnchor],
        [self.borrowButton.trailingAnchor constraintEqualToAnchor:form.trailingAnchor],
        [self.borrowButton.heightAnchor constraintEqualToConstant:40],
        [self.borrowButton.bottomAnchor constraintEqualToAnchor:form.bottomAnchor],

        [self.summaryLabel.topAnchor constraintEqualToAnchor:form.bottomAnchor constant:6],
        [self.summaryLabel.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:16],
        [self.summaryLabel.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-16],

        [self.tableView.topAnchor constraintEqualToAnchor:self.summaryLabel.bottomAnchor constant:4],
        [self.tableView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor],
        [self.tableView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [self.tableView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
    ]];

    // 点击表单区域收起键盘
    UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc]
        initWithTarget:self action:@selector(dismissKeyboard)];
    tap.cancelsTouchesInView = NO;
    [self.view addGestureRecognizer:tap];

    [self reloadData];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    [self reloadData];
}

- (void)reloadData {
    self.records = [[LBLibraryStore sharedStore] records];
    [self.tableView reloadData];

    LBLibraryStore *store = [LBLibraryStore sharedStore];
    NSDictionary *s = [store stats];
    self.summaryLabel.text = [NSString stringWithFormat:
        @"图书 %@ 种 / 在借 %@ 笔 / 逾期 %@ 笔 / 读者 %@ 人",
        s[@"titles"], s[@"active"], s[@"overdue"], s[@"readers"]];
}

- (void)dismissKeyboard {
    [self.view endEditing:YES];
}

- (UILabel *)makeLabel:(NSString *)text {
    UILabel *lbl = [[UILabel alloc] init];
    lbl.text = text;
    lbl.font = [UIFont systemFontOfSize:15];
    return lbl;
}

- (UITextField *)makeField:(NSString *)placeholder keyboard:(UIKeyboardType)type {
    UITextField *tf = [[UITextField alloc] init];
    tf.borderStyle = UITextBorderStyleRoundedRect;
    tf.placeholder = placeholder;
    tf.keyboardType = type;
    tf.font = [UIFont systemFontOfSize:15];
    return tf;
}

#pragma mark - 选择读者/图书

- (void)selectReader {
    NSArray<LBReader *> *readers = [[LBLibraryStore sharedStore] readers];
    if (readers.count == 0) {
        [self showTip:@"暂无读者，请先在「读者」页添加"];
        return;
    }
    UIAlertController *sheet = [UIAlertController alertControllerWithTitle:@"选择读者"
                                                                   message:nil
                                                            preferredStyle:UIAlertControllerStyleActionSheet];
    for (LBReader *r in readers) {
        [sheet addAction:[UIAlertAction actionWithTitle:[NSString stringWithFormat:@"%@（%@）", r.name, r.readerId]
                                                  style:UIAlertActionStyleDefault
                                                handler:^(UIAlertAction *a) {
            self.selectedReader = r;
            self.readerField.text = [NSString stringWithFormat:@"%@（%@）", r.name, r.readerId];
        }]];
    }
    [sheet addAction:[UIAlertAction actionWithTitle:@"取消" style:UIAlertActionStyleCancel handler:nil]];
    [self presentViewController:sheet animated:YES completion:nil];
}

- (void)selectBook {
    NSArray<LBBook *> *books = [[LBLibraryStore sharedStore] searchBooks:@""];
    NSMutableArray *borrowable = [NSMutableArray array];
    for (LBBook *b in books)
        if (b.availableCopies > 0) [borrowable addObject:b];
    if (borrowable.count == 0) {
        [self showTip:@"暂无可借图书"];
        return;
    }
    UIAlertController *sheet = [UIAlertController alertControllerWithTitle:@"选择图书"
                                                                   message:nil
                                                            preferredStyle:UIAlertControllerStyleActionSheet];
    for (LBBook *b in borrowable) {
        [sheet addAction:[UIAlertAction actionWithTitle:[NSString stringWithFormat:@"《%@》%@（可借%ld册）", b.title, b.author, (long)b.availableCopies]
                                                  style:UIAlertActionStyleDefault
                                                handler:^(UIAlertAction *a) {
            self.selectedBook = b;
            self.bookField.text = [NSString stringWithFormat:@"《%@》（可借%ld册）", b.title, (long)b.availableCopies];
        }]];
    }
    [sheet addAction:[UIAlertAction actionWithTitle:@"取消" style:UIAlertActionStyleCancel handler:nil]];
    [self presentViewController:sheet animated:YES completion:nil];
}

#pragma mark - 借书

- (void)borrowTapped {
    if (!self.selectedReader) {
        [self showTip:@"请先选择读者"];
        return;
    }
    if (!self.selectedBook) {
        [self showTip:@"请先选择图书"];
        return;
    }
    NSInteger days = 14;
    if (self.daysField.text.length > 0) days = self.daysField.text.integerValue;
    NSString *err = nil;
    BOOL ok = [[LBLibraryStore sharedStore] borrowBook:self.selectedBook.bookId
                                             byReader:self.selectedReader.readerId
                                                 days:days
                                                error:&err];
    if (!ok) {
        [self showTip:err ?: @"借书失败"];
        return;
    }
    [self showTip:[NSString stringWithFormat:@"借书成功：《%@》应还 %@",
                   self.selectedBook.title,
                   [LBLibraryStore dateByAddingDays:days
                                            toDate:[LBLibraryStore todayString]]]];
    self.selectedBook = nil;
    self.selectedReader = nil;
    self.readerField.text = @"";
    self.bookField.text = @"";
    [self reloadData];
}

- (void)showTip:(NSString *)message {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"提示"
                                                                   message:message
                                                            preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"好" style:UIAlertActionStyleDefault handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}

#pragma mark - UITextFieldDelegate

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField {
    if (textField == self.readerField) {
        [self selectReader];
        return NO;
    }
    if (textField == self.bookField) {
        [self selectBook];
        return NO;
    }
    return YES;
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.records.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"RecordCell"
                                                            forIndexPath:indexPath];
    LBRecord *rec = self.records[indexPath.row];
    LBLibraryStore *store = [LBLibraryStore sharedStore];
    LBBook *book = [store bookById:rec.bookId];
    LBReader *reader = [store readerById:rec.readerId];
    NSString *title = [NSString stringWithFormat:@"《%@》→ %@%@",
                       book.title, reader.name,
                       [rec isReturned] ? [NSString stringWithFormat:@"（已还 %@）", rec.returnDate] : @""];
    cell.textLabel.text = title;
    cell.textLabel.numberOfLines = 2;

    NSString *status = [rec isReturned] ? @"已还" :
        ([store recordIsOverdue:rec] ? @"逾期" : @"在借");
    UIColor *color = [store recordIsOverdue:rec] && ![rec isReturned] ?
        [UIColor systemRedColor] : [UIColor secondaryLabelColor];
    NSString *detail = [NSString stringWithFormat:@"借出 %@ ｜ 应还 %@ ｜ %@",
                        rec.borrowDate, rec.dueDate, status];
    cell.detailTextLabel.text = detail;
    cell.detailTextLabel.textColor = color;
    return cell;
}

- (UISwipeActionsConfiguration *)tableView:(UITableView *)tableView
         trailingSwipeActionsConfigurationForRowAtIndexPath:(NSIndexPath *)indexPath {
    LBRecord *rec = self.records[indexPath.row];
    if ([rec isReturned]) return nil;
    UIContextualAction *ret = [UIContextualAction
        contextualActionWithStyle:UIContextualActionStyleNormal
                            title:@"归还"
                          handler:^(UIContextualAction *action, __kindof UIView *sourceView,
                                    void (^completion)(BOOL)) {
        NSString *err = nil;
        if ([[LBLibraryStore sharedStore] returnBook:rec.bookId
                                            byReader:rec.readerId
                                               error:&err]) {
            [self reloadData];
            completion(YES);
        } else {
            completion(NO);
            [self showTip:err ?: @"归还失败"];
        }
    }];
    ret.backgroundColor = [UIColor systemGreenColor];
    return [UISwipeActionsConfiguration configurationWithActions:@[ret]];
}

@end
