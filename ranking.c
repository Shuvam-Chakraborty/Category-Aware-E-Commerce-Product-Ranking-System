#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

#define MAX_NAME_LEN 100
#define MAX_CATEGORY_LEN 50
#define MAX_PRODUCTS 100
#define MAX_CATEGORIES 20

/* Category weights structure */
typedef struct {
    char category_name[MAX_CATEGORY_LEN];
    float rating_weight;
    float review_weight;
    float sales_weight;
    float price_weight;
} CategoryWeights;

/* Product structure */
typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char category[MAX_CATEGORY_LEN];
    float rating;
    int num_reviews;
    int sales;
    float price;
    float ranking_score;
} Product;

/* Max Heap structure */
typedef struct {
    Product *products[MAX_PRODUCTS];
    int size;
} MaxHeap;

/* Category Heap structure */
typedef struct {
    char category_name[MAX_CATEGORY_LEN];
    MaxHeap *heap;
    CategoryWeights weights;
} CategoryHeap;

/* Global category manager */
typedef struct {
    CategoryHeap categories[MAX_CATEGORIES];
    int num_categories;
} CategoryManager;

/* Function prototypes */
MaxHeap* createHeap();
void swap(Product **a, Product **b);
void heapifyUp(MaxHeap *heap, int index);
void heapifyDown(MaxHeap *heap, int index);
void insertProduct(MaxHeap *heap, Product *product);
Product* extractMax(MaxHeap *heap);
float calculateRankingScore(Product *product, CategoryWeights *weights);
Product* createProduct(int id, const char *name, const char *category, float rating, int reviews, int sales, float price);
void displayProduct(Product *product, int rank);
void displayTopN(MaxHeap *heap, int n, const char *category_name);
void displayAllProducts(MaxHeap *heap);
void freeHeap(MaxHeap *heap);
void printLine(int length);
void printHeader(const char *title);
void clearInputBuffer();
void toLowerCase(char *str);
int getNextId();
void clearScreen();

/* Category Manager functions */
CategoryManager* createCategoryManager();
CategoryHeap* getCategoryHeap(CategoryManager *manager, const char *category);
void insertProductToCategory(CategoryManager *manager, Product *product);
void displayTopProductsByCategory(CategoryManager *manager, int n);
void displayAllCategories(CategoryManager *manager);
void searchByCategory(CategoryManager *manager, const char *category);
void searchByName(CategoryManager *manager, const char *keyword);
void updateProductInCategory(CategoryManager *manager, int id);
void deleteProductFromCategory(CategoryManager *manager, int id);
void displayCategoryStatistics(CategoryManager *manager);
void displayGlobalStatistics(CategoryManager *manager);
void addSampleProducts(CategoryManager *manager);
void freeCategoryManager(CategoryManager *manager);
void initializeDefaultWeights(CategoryWeights *weights, const char *category);
void displayCategoryWeights(CategoryManager *manager);
void customizeWeights(CategoryManager *manager);
void exportToCSV(CategoryManager *manager, const char *filename);

/* Global ID counter */
int global_id = 1;

/* Create a new max heap */
MaxHeap* createHeap() {
    MaxHeap *heap = (MaxHeap*)malloc(sizeof(MaxHeap));
    if (heap == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    heap->size = 0;
    return heap;
}

/* Swap two products */
void swap(Product **a, Product **b) {
    Product *temp = *a;
    *a = *b;
    *b = temp;
}

/* Initialize default weights based on category */
void initializeDefaultWeights(CategoryWeights *weights, const char *category) {
    char cat_lower[MAX_CATEGORY_LEN];
    strncpy(cat_lower, category, MAX_CATEGORY_LEN - 1);
    cat_lower[MAX_CATEGORY_LEN - 1] = '\0';
    toLowerCase(cat_lower);

    strncpy(weights->category_name, category, MAX_CATEGORY_LEN - 1);
    weights->category_name[MAX_CATEGORY_LEN - 1] = '\0';

    /* Category-specific weights */
    if (strstr(cat_lower, "electronic") != NULL ||
        strstr(cat_lower, "gadget") != NULL ||
        strstr(cat_lower, "computer") != NULL ||
        strstr(cat_lower, "electronics") != NULL) {
        weights->rating_weight = 0.40f;
        weights->review_weight = 0.30f;
        weights->sales_weight = 0.20f;
        weights->price_weight = 0.10f;
    } else if (strstr(cat_lower, "fashion") != NULL ||
               strstr(cat_lower, "apparel") != NULL ||
               strstr(cat_lower, "clothing") != NULL ||
               strstr(cat_lower, "accessories") != NULL) {
        weights->rating_weight = 0.20f;
        weights->review_weight = 0.40f;
        weights->sales_weight = 0.30f;
        weights->price_weight = 0.10f;
    } else if (strstr(cat_lower, "book") != NULL ||
               strstr(cat_lower, "media") != NULL) {
        weights->rating_weight = 0.45f;
        weights->review_weight = 0.35f;
        weights->sales_weight = 0.15f;
        weights->price_weight = 0.05f;
    } else if (strstr(cat_lower, "audio") != NULL ||
               strstr(cat_lower, "speaker") != NULL ||
               strstr(cat_lower, "headphone") != NULL) {
        weights->rating_weight = 0.50f;
        weights->review_weight = 0.25f;
        weights->sales_weight = 0.15f;
        weights->price_weight = 0.10f;
    } else if (strstr(cat_lower, "storage") != NULL) {
        weights->rating_weight = 0.35f;
        weights->review_weight = 0.25f;
        weights->sales_weight = 0.25f;
        weights->price_weight = 0.15f;
    } else {
        /* Default weights */
        weights->rating_weight = 0.35f;
        weights->review_weight = 0.25f;
        weights->sales_weight = 0.30f;
        weights->price_weight = 0.10f;
    }
}

/* Calculate ranking score with category-specific weighted formula */
float calculateRankingScore(Product *product, CategoryWeights *weights) {
    float rating_score = (product->rating / 5.0f) * weights->rating_weight;
    float review_score = 0.0f;
    float sales_score = 0.0f;
    float price_factor = 0.0f;

    if (product->num_reviews > 0) {
        review_score = (log10((float)product->num_reviews + 1.0f) / log10(10000.0f)) * weights->review_weight;
        if (review_score > weights->review_weight) review_score = weights->review_weight;
    }

    if (product->sales > 0) {
        sales_score = (log10((float)product->sales + 1.0f) / log10(100000.0f)) * weights->sales_weight;
        if (sales_score > weights->sales_weight) sales_score = weights->sales_weight;
    }

    if (product->price > 0) {
        price_factor = (1.0f / (1.0f + product->price / 1000.0f)) * weights->price_weight;
    }

    return rating_score + review_score + sales_score + price_factor;
}

/* Heapify up */
void heapifyUp(MaxHeap *heap, int index) {
    int parent;
    if (index == 0) return;

    parent = (index - 1) / 2;

    if (heap->products[index]->ranking_score > heap->products[parent]->ranking_score) {
        swap(&heap->products[index], &heap->products[parent]);
        heapifyUp(heap, parent);
    }
}

/* Heapify down */
void heapifyDown(MaxHeap *heap, int index) {
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < heap->size &&
        heap->products[left]->ranking_score > heap->products[largest]->ranking_score) {
        largest = left;
    }

    if (right < heap->size &&
        heap->products[right]->ranking_score > heap->products[largest]->ranking_score) {
        largest = right;
    }

    if (largest != index) {
        swap(&heap->products[index], &heap->products[largest]);
        heapifyDown(heap, largest);
    }
}

/* Insert product into heap */
void insertProduct(MaxHeap *heap, Product *product) {
    if (heap->size >= MAX_PRODUCTS) {
        printf("\n[ERROR] Heap is full! Cannot add more products.\n");
        free(product);
        return;
    }

    heap->products[heap->size] = product;
    heapifyUp(heap, heap->size);
    heap->size++;
}

/* Extract maximum */
Product* extractMax(MaxHeap *heap) {
    Product *max;
    if (heap->size == 0) {
        return NULL;
    }

    max = heap->products[0];
    heap->products[0] = heap->products[heap->size - 1];
    heap->size--;

    if (heap->size > 0) {
        heapifyDown(heap, 0);
    }

    return max;
}

/* Create a new product */
Product* createProduct(int id, const char *name, const char *category, float rating, int reviews, int sales, float price) {
    Product *product = (Product*)malloc(sizeof(Product));
    if (product == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    product->id = id;
    strncpy(product->name, name, MAX_NAME_LEN - 1);
    product->name[MAX_NAME_LEN - 1] = '\0';
    strncpy(product->category, category, MAX_CATEGORY_LEN - 1);
    product->category[MAX_CATEGORY_LEN - 1] = '\0';
    product->rating = rating;
    product->num_reviews = reviews;
    product->sales = sales;
    product->price = price;
    product->ranking_score = 0.0f;
    return product;
}

/* Convert string to lowercase */
void toLowerCase(char *str) {
    int i;
    for (i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

/* Print a line */
void printLine(int length) {
    int i;
    for (i = 0; i < length; i++) {
        printf("=");
    }
    printf("\n");
}

/* Print header */
void printHeader(const char *title) {
    int len = strlen(title);
    int total = 100;
    int padding = (total - len) / 2;
    int i;

    printf("\n");
    printLine(total);
    for (i = 0; i < padding; i++) printf(" ");
    printf("%s\n", title);
    printLine(total);
}

/* Display product details */
void displayProduct(Product *product, int rank) {
    printf("  [Rank #%d] %s\n", rank, product->name);
    printf("     ID: %-4d | Category: %-15s | Price: $%.2f\n",
           product->id, product->category, product->price);
    printf("     Rating: %.1f/5.0 | Reviews: %-6d | Sales: %-7d | Score: %.4f\n",
           product->rating, product->num_reviews, product->sales, product->ranking_score);
    printf("\n");
}

/* Display top N products */
void displayTopN(MaxHeap *heap, int n, const char *category_name) {
    MaxHeap *tempHeap;
    int count;
    int i;
    Product *product;
    Product *copy;

    if (heap->size == 0) {
        printf("\n[INFO] No products available in category: %s\n", category_name);
        return;
    }

    tempHeap = createHeap();
    for (i = 0; i < heap->size; i++) {
        copy = (Product*)malloc(sizeof(Product));
        if (copy == NULL) {
            printf("Memory allocation failed!\n");
            exit(1);
        }
        memcpy(copy, heap->products[i], sizeof(Product));
        tempHeap->products[i] = copy;
    }
    tempHeap->size = heap->size;

    count = (n < tempHeap->size) ? n : tempHeap->size;
    printf("\n-- Category: %s - Showing top %d of %d products:\n\n", category_name, count, heap->size);

    for (i = 0; i < count; i++) {
        product = extractMax(tempHeap);
        displayProduct(product, i + 1);
        free(product);
    }

    free(tempHeap);
}

/* Create category manager */
CategoryManager* createCategoryManager() {
    CategoryManager *manager = (CategoryManager*)malloc(sizeof(CategoryManager));
    if (manager == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    manager->num_categories = 0;
    return manager;
}

/* Get or create category heap */
CategoryHeap* getCategoryHeap(CategoryManager *manager, const char *category) {
    int i;
    char cat_lower[MAX_CATEGORY_LEN];
    char existing_lower[MAX_CATEGORY_LEN];

    strncpy(cat_lower, category, MAX_CATEGORY_LEN - 1);
    cat_lower[MAX_CATEGORY_LEN - 1] = '\0';
    toLowerCase(cat_lower);

    /* Check if category exists */
    for (i = 0; i < manager->num_categories; i++) {
        strncpy(existing_lower, manager->categories[i].category_name, MAX_CATEGORY_LEN - 1);
        existing_lower[MAX_CATEGORY_LEN - 1] = '\0';
        toLowerCase(existing_lower);

        if (strcmp(cat_lower, existing_lower) == 0) {
            return &manager->categories[i];
        }
    }

    /* Create new category */
    if (manager->num_categories >= MAX_CATEGORIES) {
        printf("\n[ERROR] Maximum categories reached!\n");
        return NULL;
    }

    strncpy(manager->categories[manager->num_categories].category_name, category, MAX_CATEGORY_LEN - 1);
    manager->categories[manager->num_categories].category_name[MAX_CATEGORY_LEN - 1] = '\0';
    manager->categories[manager->num_categories].heap = createHeap();
    initializeDefaultWeights(&manager->categories[manager->num_categories].weights, category);

    manager->num_categories++;
    printf("\n[INFO] New category created: %s\n", category);

    return &manager->categories[manager->num_categories - 1];
}

/* Insert product to appropriate category */
void insertProductToCategory(CategoryManager *manager, Product *product) {
    CategoryHeap *catHeap = getCategoryHeap(manager, product->category);
    if (catHeap == NULL) return;

    product->ranking_score = calculateRankingScore(product, &catHeap->weights);
    insertProduct(catHeap->heap, product);
    printf("[SUCCESS] Product added to '%s' category! (ID: %d)\n", product->category, product->id);
}

/* Display top products by category */
void displayTopProductsByCategory(CategoryManager *manager, int n) {
    int i;

    if (manager->num_categories == 0) {
        printf("\n[INFO] No categories available.\n");
        return;
    }

    printHeader("TOP PRODUCTS BY CATEGORY");

    for (i = 0; i < manager->num_categories; i++) {
        displayTopN(manager->categories[i].heap, n, manager->categories[i].category_name);
        if (i < manager->num_categories - 1) {
            printLine(100);
        }
    }

    printLine(100);
}

/* Display all categories */
void displayAllCategories(CategoryManager *manager) {
    int i;

    if (manager->num_categories == 0) {
        printf("\n[INFO] No categories available.\n");
        return;
    }

    printHeader("ALL CATEGORIES");
    printf("\n");

    for (i = 0; i < manager->num_categories; i++) {
        printf("  [%d] %s (%d products)\n",
               i + 1,
               manager->categories[i].category_name,
               manager->categories[i].heap->size);
    }

    printf("\n");
    printLine(100);
}

/* Search by category */
void searchByCategory(CategoryManager *manager, const char *category) {
    int i;
    char cat_lower[MAX_CATEGORY_LEN];
    char search_lower[MAX_CATEGORY_LEN];
    int found = 0;

    strncpy(search_lower, category, MAX_CATEGORY_LEN - 1);
    search_lower[MAX_CATEGORY_LEN - 1] = '\0';
    toLowerCase(search_lower);

    printHeader("SEARCH RESULTS BY CATEGORY");
    printf("\nSearching for: %s\n", category);

    for (i = 0; i < manager->num_categories; i++) {
        strncpy(cat_lower, manager->categories[i].category_name, MAX_CATEGORY_LEN - 1);
        cat_lower[MAX_CATEGORY_LEN - 1] = '\0';
        toLowerCase(cat_lower);

        if (strstr(cat_lower, search_lower) != NULL) {
            displayTopN(manager->categories[i].heap, manager->categories[i].heap->size,
                       manager->categories[i].category_name);
            found = 1;
        }
    }

    if (!found) {
        printf("\n[INFO] No products found in category: %s\n", category);
    }

    printLine(100);
}

/* Search by name */
void searchByName(CategoryManager *manager, const char *keyword) {
    int i, j;
    char searchKey[MAX_NAME_LEN];
    char productName[MAX_NAME_LEN];
    int found = 0;

    strncpy(searchKey, keyword, MAX_NAME_LEN - 1);
    searchKey[MAX_NAME_LEN - 1] = '\0';
    toLowerCase(searchKey);

    printHeader("SEARCH RESULTS BY NAME");
    printf("\nSearching for: %s\n\n", keyword);

    for (i = 0; i < manager->num_categories; i++) {
        for (j = 0; j < manager->categories[i].heap->size; j++) {
            strncpy(productName, manager->categories[i].heap->products[j]->name, MAX_NAME_LEN - 1);
            productName[MAX_NAME_LEN - 1] = '\0';
            toLowerCase(productName);

            if (strstr(productName, searchKey) != NULL) {
                displayProduct(manager->categories[i].heap->products[j], j + 1);
                found = 1;
            }
        }
    }

    if (!found) {
        printf("[INFO] No products found matching: %s\n", keyword);
    }

    printLine(100);
}

/* Update product in category */
void updateProductInCategory(CategoryManager *manager, int id) {
    int i, j;
    int choice;
    float newRating;
    int newReviews, newSales;
    float newPrice;
    Product *product = NULL;
    CategoryHeap *catHeap = NULL;

    for (i = 0; i < manager->num_categories; i++) {
        for (j = 0; j < manager->categories[i].heap->size; j++) {
            if (manager->categories[i].heap->products[j]->id == id) {
                product = manager->categories[i].heap->products[j];
                catHeap = &manager->categories[i];
                break;
            }
        }
        if (product != NULL) break;
    }

    if (product == NULL) {
        printf("\n[ERROR] Product ID %d not found!\n", id);
        return;
    }

    printf("\n--- Update Product: %s ---\n", product->name);
    printf("Category: %s\n", product->category);
    printf("1. Update Rating (Current: %.1f)\n", product->rating);
    printf("2. Update Reviews (Current: %d)\n", product->num_reviews);
    printf("3. Update Sales (Current: %d)\n", product->sales);
    printf("4. Update Price (Current: $%.2f)\n", product->price);
    printf("Enter choice: ");
    scanf("%d", &choice);
    clearInputBuffer();

    switch(choice) {
        case 1:
            printf("Enter new rating (0-5): ");
            scanf("%f", &newRating);
            clearInputBuffer();
            if (newRating >= 0 && newRating <= 5) {
                product->rating = newRating;
                printf("[SUCCESS] Rating updated!\n");
            } else {
                printf("[ERROR] Invalid rating!\n");
                return;
            }
            break;
        case 2:
            printf("Enter new review count: ");
            scanf("%d", &newReviews);
            clearInputBuffer();
            if (newReviews >= 0) {
                product->num_reviews = newReviews;
                printf("[SUCCESS] Reviews updated!\n");
            } else {
                printf("[ERROR] Invalid count!\n");
                return;
            }
            break;
        case 3:
            printf("Enter new sales count: ");
            scanf("%d", &newSales);
            clearInputBuffer();
            if (newSales >= 0) {
                product->sales = newSales;
                printf("[SUCCESS] Sales updated!\n");
            } else {
                printf("[ERROR] Invalid count!\n");
                return;
            }
            break;
        case 4:
            printf("Enter new price: $");
            scanf("%f", &newPrice);
            clearInputBuffer();
            if (newPrice >= 0) {
                product->price = newPrice;
                printf("[SUCCESS] Price updated!\n");
            } else {
                printf("[ERROR] Invalid price!\n");
                return;
            }
            break;
        default:
            printf("[ERROR] Invalid choice!\n");
            return;
    }

    /* Recalculate score and rebuild heap */
    product->ranking_score = calculateRankingScore(product, &catHeap->weights);
    for (j = (catHeap->heap->size / 2) - 1; j >= 0; j--) {
        heapifyDown(catHeap->heap, j);
    }

    printf("[INFO] Product re-ranked in category '%s'\n", catHeap->category_name);
}

/* Delete product from category */
void deleteProductFromCategory(CategoryManager *manager, int id) {
    int i, j;
    int found = -1;
    CategoryHeap *catHeap = NULL;

    for (i = 0; i < manager->num_categories; i++) {
        for (j = 0; j < manager->categories[i].heap->size; j++) {
            if (manager->categories[i].heap->products[j]->id == id) {
                found = j;
                catHeap = &manager->categories[i];
                break;
            }
        }
        if (found != -1) break;
    }

    if (found == -1) {
        printf("\n[ERROR] Product ID %d not found!\n", id);
        return;
    }

    printf("\n[INFO] Deleting product: %s from category '%s'\n",
           catHeap->heap->products[found]->name, catHeap->category_name);
    free(catHeap->heap->products[found]);

    catHeap->heap->products[found] = catHeap->heap->products[catHeap->heap->size - 1];
    catHeap->heap->size--;

    for (j = (catHeap->heap->size / 2) - 1; j >= 0; j--) {
        heapifyDown(catHeap->heap, j);
    }

    printf("[SUCCESS] Product deleted successfully!\n");
}

/* Display category statistics */
void displayCategoryStatistics(CategoryManager *manager) {
    int i, j;

    if (manager->num_categories == 0) {
        printf("\n[INFO] No categories available.\n");
        return;
    }

    printHeader("CATEGORY-WISE STATISTICS");

    for (i = 0; i < manager->num_categories; i++) {
        MaxHeap *heap = manager->categories[i].heap;

        if (heap->size == 0) continue;

        float avgRating = 0;
        int totalReviews = 0;
        int totalSales = 0;
        float totalPrice = 0;
        Product *maxSales = NULL;
        Product *maxRating = NULL;

        for (j = 0; j < heap->size; j++) {
            avgRating += heap->products[j]->rating;
            totalReviews += heap->products[j]->num_reviews;
            totalSales += heap->products[j]->sales;
            totalPrice += heap->products[j]->price;

            if (maxSales == NULL || heap->products[j]->sales > maxSales->sales) {
                maxSales = heap->products[j];
            }

            if (maxRating == NULL || heap->products[j]->rating > maxRating->rating) {
                maxRating = heap->products[j];
            }
        }

        printf("\n-- Category: %s\n", manager->categories[i].category_name);
        printf("   Total Products: %d\n", heap->size);
        printf("   Average Rating: %.2f/5.0\n", avgRating / heap->size);
        printf("   Total Reviews: %d\n", totalReviews);
        printf("   Total Sales: %d units\n", totalSales);
        printf("   Average Price: $%.2f\n", totalPrice / heap->size);

        if (maxSales) {
            printf("   Best Seller: %s (%d units)\n", maxSales->name, maxSales->sales);
        }

        if (maxRating) {
            printf("   Highest Rated: %s (%.1f/5.0)\n", maxRating->name, maxRating->rating);
        }
    }

    printf("\n");
    printLine(100);
}

/* Display global statistics */
void displayGlobalStatistics(CategoryManager *manager) {
    int i, j;
    int totalProducts = 0;
    float totalRating = 0;
    int totalReviews = 0;
    int totalSales = 0;
    float totalPrice = 0;
    Product *globalMaxSales = NULL;
    Product *globalMaxRating = NULL;

    if (manager->num_categories == 0) {
        printf("\n[INFO] No data available.\n");
        return;
    }

    printHeader("GLOBAL SYSTEM STATISTICS");

    for (i = 0; i < manager->num_categories; i++) {
        MaxHeap *heap = manager->categories[i].heap;

        for (j = 0; j < heap->size; j++) {
            totalProducts++;
            totalRating += heap->products[j]->rating;
            totalReviews += heap->products[j]->num_reviews;
            totalSales += heap->products[j]->sales;
            totalPrice += heap->products[j]->price;

            if (globalMaxSales == NULL || heap->products[j]->sales > globalMaxSales->sales) {
                globalMaxSales = heap->products[j];
            }

            if (globalMaxRating == NULL || heap->products[j]->rating > globalMaxRating->rating) {
                globalMaxRating = heap->products[j];
            }
        }
    }

    if (totalProducts == 0) {
        printf("\n[INFO] No products in system.\n");
        printLine(100);
        return;
    }

    printf("\n-- OVERALL METRICS:\n");
    printf("   Total Categories: %d\n", manager->num_categories);
    printf("   Total Products: %d\n", totalProducts);
    printf("   Average Rating: %.2f/5.0\n", totalRating / totalProducts);
    printf("   Total Reviews: %d\n", totalReviews);
    printf("   Total Sales: %d units\n", totalSales);
    printf("   Average Price: $%.2f\n", totalPrice / totalProducts);

    if (globalMaxSales) {
        printf("\n-- Global Best Seller: %s (%d units) [%s]\n",
               globalMaxSales->name, globalMaxSales->sales, globalMaxSales->category);
    }

    if (globalMaxRating) {
        printf("-- Global Highest Rated: %s (%.1f/5.0) [%s]\n",
               globalMaxRating->name, globalMaxRating->rating, globalMaxRating->category);
    }

    printf("\n");
    printLine(100);
}

/* Display category weights */
void displayCategoryWeights(CategoryManager *manager) {
    int i;

    if (manager->num_categories == 0) {
        printf("\n[INFO] No categories available.\n");
        return;
    }

    printHeader("CATEGORY WEIGHTING CONFIGURATION");
    printf("\n");

    for (i = 0; i < manager->num_categories; i++) {
        CategoryWeights *w = &manager->categories[i].weights;
        printf("-- %s:\n", manager->categories[i].category_name);
        printf("   Rating: %.0f%% | Reviews: %.0f%% | Sales: %.0f%% | Price: %.0f%%\n\n",
               w->rating_weight * 100, w->review_weight * 100,
               w->sales_weight * 100, w->price_weight * 100);
    }

    printLine(100);
}

/* Customize weights for a category */
void customizeWeights(CategoryManager *manager) {
    int i, catChoice;
    float r, rev, s, p, total;

    if (manager->num_categories == 0) {
        printf("\n[INFO] No categories available.\n");
        return;
    }

    printf("\n--- Customize Category Weights ---\n");
    for (i = 0; i < manager->num_categories; i++) {
        printf("[%d] %s\n", i + 1, manager->categories[i].category_name);
    }
    printf("Select category: ");
    scanf("%d", &catChoice);
    clearInputBuffer();

    if (catChoice < 1 || catChoice > manager->num_categories) {
        printf("[ERROR] Invalid category!\n");
        return;
    }

    CategoryWeights *w = &manager->categories[catChoice - 1].weights;

    printf("\nCurrent weights for '%s':\n", manager->categories[catChoice - 1].category_name);
    printf("Rating: %.0f%%, Reviews: %.0f%%, Sales: %.0f%%, Price: %.0f%%\n\n",
           w->rating_weight * 100, w->review_weight * 100,
           w->sales_weight * 100, w->price_weight * 100);

    printf("Enter new weights (must sum to 100):\n");
    printf("Rating weight (0-100): ");
    scanf("%f", &r);
    printf("Reviews weight (0-100): ");
    scanf("%f", &rev);
    printf("Sales weight (0-100): ");
    scanf("%f", &s);
    printf("Price weight (0-100): ");
    scanf("%f", &p);
    clearInputBuffer();

    total = r + rev + s + p;

    if (fabs(total - 100.0f) > 0.01f) {
        printf("[ERROR] Weights must sum to 100 (current sum: %.1f)\n", total);
        return;
    }

    w->rating_weight = r / 100.0f;
    w->review_weight = rev / 100.0f;
    w->sales_weight = s / 100.0f;
    w->price_weight = p / 100.0f;

    /* Recalculate all scores in this category */
    MaxHeap *heap = manager->categories[catChoice - 1].heap;
    for (i = 0; i < heap->size; i++) {
        heap->products[i]->ranking_score = calculateRankingScore(heap->products[i], w);
    }

    /* Rebuild heap */
    for (i = (heap->size / 2) - 1; i >= 0; i--) {
        heapifyDown(heap, i);
    }

    printf("[SUCCESS] Weights updated and products re-ranked!\n");
}

/* Export data to CSV */
void exportToCSV(CategoryManager *manager, const char *filename) {
    FILE *file;
    int i, j;

    file = fopen(filename, "w");
    if (file == NULL) {
        printf("\n[ERROR] Could not create file: %s\n", filename);
        return;
    }

    fprintf(file, "ID,Name,Category,Rating,Reviews,Sales,Price,RankingScore\n");

    for (i = 0; i < manager->num_categories; i++) {
        MaxHeap *heap = manager->categories[i].heap;
        for (j = 0; j < heap->size; j++) {
            Product *p = heap->products[j];
            fprintf(file, "%d,\"%s\",\"%s\",%.1f,%d,%d,%.2f,%.4f\n",
                    p->id, p->name, p->category, p->rating,
                    p->num_reviews, p->sales, p->price, p->ranking_score);
        }
    }

    fclose(file);
    printf("\n[SUCCESS] Data exported to '%s'\n", filename);
}

/* Get next available ID */
int getNextId() {
    return global_id++;
}

/* Clear input buffer */
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Clear screen (unix) */
void clearScreen() {
    printf("\nPress Enter to continue...");
    getchar();  // waits for Enter key
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* Add sample products - 10 products per category */
void addSampleProducts(CategoryManager *manager) {
    /* Electronics - 10 items */
    insertProductToCategory(manager, createProduct(getNextId(), "Wireless Headphones Pro", "Electronics", 4.5f, 2500, 15000, 79.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Smart Watch X1", "Electronics", 4.8f, 5000, 25000, 199.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "USB-C Cable Premium", "Electronics", 4.2f, 800, 50000, 12.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Mechanical Keyboard RGB", "Electronics", 4.7f, 3200, 8000, 129.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Gaming Mouse Elite", "Electronics", 4.9f, 4500, 12000, 59.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Webcam 4K Ultra", "Electronics", 4.3f, 650, 3200, 89.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Bluetooth Adapter", "Electronics", 4.0f, 420, 7000, 14.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Portable Charger 20000mAh", "Electronics", 4.4f, 2100, 22000, 39.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Smart Home Hub", "Electronics", 4.1f, 900, 4500, 99.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Noise Cancelling Earphones", "Electronics", 4.6f, 1800, 9000, 69.99f));

    /* Accessories - 10 items */
    insertProductToCategory(manager, createProduct(getNextId(), "Phone Case Premium Leather", "Accessories", 3.8f, 1200, 35000, 24.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Laptop Stand Aluminum", "Accessories", 4.6f, 1800, 6000, 39.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Screen Protector Tempered Glass", "Accessories", 4.1f, 2100, 28000, 9.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Wireless Charger Pad", "Accessories", 4.2f, 900, 14000, 19.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "USB Hub 4-Port", "Accessories", 4.0f, 650, 8000, 22.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Car Phone Mount", "Accessories", 4.3f, 1300, 12000, 14.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Reusable Cable Ties", "Accessories", 4.5f, 300, 4000, 6.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Travel Adapter Universal", "Accessories", 4.4f, 1100, 9000, 18.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Key Organizer Steel", "Accessories", 4.1f, 420, 3500, 12.49f));
    insertProductToCategory(manager, createProduct(getNextId(), "Camera Cleaning Kit", "Accessories", 4.2f, 290, 2600, 9.49f));

    /* Storage - 10 items */
    insertProductToCategory(manager, createProduct(getNextId(), "Portable SSD 1TB", "Storage", 4.4f, 900, 4500, 149.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "USB Flash Drive 128GB", "Storage", 4.0f, 1500, 18000, 19.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "External HDD 2TB", "Storage", 4.3f, 1100, 7500, 79.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "MicroSD 256GB", "Storage", 4.5f, 800, 12000, 39.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "NAS Home 4-Bay", "Storage", 4.2f, 220, 800, 299.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "USB 3.0 External Enclosure", "Storage", 4.0f, 150, 2400, 29.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Cloud Backup Subscription 1yr", "Storage", 4.6f, 600, 5000, 59.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "M.2 NVMe 512GB", "Storage", 4.7f, 1400, 9000, 89.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "External SSD 500GB", "Storage", 4.3f, 700, 4200, 99.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Data Recovery Service", "Storage", 4.1f, 90, 200, 129.99f));

    /* Audio - 10 items */
    insertProductToCategory(manager, createProduct(getNextId(), "Bluetooth Speaker Mini", "Audio", 4.1f, 3800, 22000, 34.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Earbuds Pro Noise Cancelling", "Audio", 4.7f, 4200, 16000, 119.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Studio Headphones Professional", "Audio", 4.9f, 1800, 5500, 249.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Home Theater Soundbar", "Audio", 4.2f, 900, 3200, 199.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Portable Recorder", "Audio", 4.0f, 210, 900, 89.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Audiophile Cables", "Audio", 3.9f, 120, 400, 29.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Wireless Over-Ear", "Audio", 4.4f, 800, 7200, 139.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Bluetooth Transmitter", "Audio", 4.1f, 430, 2100, 24.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Recording Microphone USB", "Audio", 4.5f, 1100, 6000, 79.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Noise Reduction Foam", "Audio", 4.0f, 95, 500, 12.99f));

    /* Books - 10 items */
    insertProductToCategory(manager, createProduct(getNextId(), "Data Structures and Algorithms", "Books", 4.8f, 3500, 12000, 45.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Clean Code", "Books", 4.9f, 5200, 18000, 39.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Design Patterns", "Books", 4.7f, 2800, 9500, 54.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Machine Learning Intro", "Books", 4.6f, 2100, 7200, 49.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Deep Learning Handbook", "Books", 4.5f, 1700, 4800, 59.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Operating Systems", "Books", 4.4f, 900, 3000, 44.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Computer Networks", "Books", 4.3f, 1100, 3500, 41.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Database Systems", "Books", 4.2f, 800, 2500, 46.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Algorithms in C", "Books", 4.1f, 600, 1900, 34.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Probability for Engineers", "Books", 4.0f, 400, 1200, 29.99f));

    /* Fashion - 10 items */
    insertProductToCategory(manager, createProduct(getNextId(), "Cotton T-Shirt Classic", "Fashion", 4.0f, 5500, 45000, 19.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Denim Jeans Slim Fit", "Fashion", 4.2f, 4200, 32000, 49.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Sports Shoes Running", "Fashion", 4.5f, 6800, 28000, 89.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Hooded Sweatshirt", "Fashion", 4.1f, 2400, 15000, 39.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Formal Shirt Slim", "Fashion", 4.0f, 1300, 9000, 29.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Leather Belt", "Fashion", 4.3f, 800, 5000, 15.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Casual Sneakers", "Fashion", 4.2f, 2100, 11000, 59.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Sunglasses UV400", "Fashion", 4.0f, 900, 7000, 24.99f));
    insertProductToCategory(manager, createProduct(getNextId(), "Wool Scarf Winter", "Fashion", 4.4f, 400, 3000, 19.49f));
    insertProductToCategory(manager, createProduct(getNextId(), "Backpack Travel", "Fashion", 4.5f, 1500, 10000, 49.99f));
}

/* Display menu */
void displayMenu() {
    printf("\n");
    printLine(100);
    printf("                     CATEGORY-AWARE E-COMMERCE PRODUCT RANKING SYSTEM\n");
    printLine(100);
    printf("\n");
    printf("  >> PRODUCT MANAGEMENT\n");
    printf("  [1]  Add New Product\n");
    printf("  [2]  Update Product\n");
    printf("  [3]  Delete Product\n");
    printf("\n");
    printf("  >> VIEW & SEARCH\n");
    printf("  [4]  View Top N Products (Per Category)\n");
    printf("  [5]  View All Categories\n");
    printf("  [6]  Search by Category\n");
    printf("  [7]  Search by Product Name\n");
    printf("\n");
    printf("  >> ANALYTICS\n");
    printf("  [8]  Category-wise Statistics\n");
    printf("  [9]  Global Statistics\n");
    printf("  [10] View Category Weights\n");
    printf("\n");
    printf("  >> CONFIGURATION\n");
    printf("  [11] Customize Category Weights\n");
    printf("  [12] Export Data to CSV\n");
    printf("  [13] Load Sample Products (10 items per category)\n");
    printf("\n");
    printf("  [0]  Exit System\n");
    printf("\n");
    printLine(100);
    printf("\nEnter your choice: ");
}

/* Free heap memory */
void freeHeap(MaxHeap *heap) {
    int i;
    for (i = 0; i < heap->size; i++) {
        free(heap->products[i]);
    }
    free(heap);
}

/* Free category manager */
void freeCategoryManager(CategoryManager *manager) {
    int i;
    for (i = 0; i < manager->num_categories; i++) {
        freeHeap(manager->categories[i].heap);
    }
    free(manager);
}

/* Main function */
int main() {
    CategoryManager *manager;
    int choice;
    int n, id;
    char name[MAX_NAME_LEN];
    char category[MAX_CATEGORY_LEN];
    char keyword[MAX_NAME_LEN];
    char filename[100];
    float rating;
    int reviews, sales;
    float price;

    manager = createCategoryManager();

    printf("\n");
    printLine(100);
    printf("           WELCOME TO CATEGORY-AWARE E-COMMERCE PRODUCT RANKING SYSTEM\n");
    printf("                         Powered by Max-Heap & Priority Queue\n");
    printLine(100);
    printf("\n[INFO] System initialized successfully!\n");
    printf("[TIP]  Products are ranked within their categories for fair comparison\n");
    printf("[TIP]  Start by loading sample products (Option 13) or add your own!\n");

    while (1) {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("\n[ERROR] Invalid input. Try again.\n");
            clearScreen();
            continue;
        }
        clearInputBuffer();

        switch(choice) {
            case 1:
                printf("\n--- Add New Product ---\n");
                printf("Product Name: ");
                fgets(name, MAX_NAME_LEN, stdin);
                name[strcspn(name, "\n")] = 0;

                printf("Category: ");
                fgets(category, MAX_CATEGORY_LEN, stdin);
                category[strcspn(category, "\n")] = 0;

                printf("Rating (0-5): ");
                scanf("%f", &rating);

                printf("Number of Reviews: ");
                scanf("%d", &reviews);

                printf("Sales Count: ");
                scanf("%d", &sales);

                printf("Price: $");
                scanf("%f", &price);
                clearInputBuffer();

                if (rating < 0 || rating > 5 || reviews < 0 || sales < 0 || price < 0) {
                    printf("[ERROR] Invalid input values!\n");
                } else {
                    insertProductToCategory(manager, createProduct(getNextId(), name, category, rating, reviews, sales, price));
                }
                clearScreen();
                break;

            case 2:
                printf("\nEnter Product ID to update: ");
                if (scanf("%d", &id) != 1) {
                    clearInputBuffer();
                    printf("[ERROR] Invalid input.\n");
                } else {
                    clearInputBuffer();
                    updateProductInCategory(manager, id);
                }
                clearScreen();
                break;

            case 3:
                printf("\nEnter Product ID to delete: ");
                if (scanf("%d", &id) != 1) {
                    clearInputBuffer();
                    printf("[ERROR] Invalid input.\n");
                } else {
                    clearInputBuffer();
                    deleteProductFromCategory(manager, id);
                }
                clearScreen();
                break;

            case 4:
                printf("\nHow many top products per category? ");
                if (scanf("%d", &n) != 1) {
                    clearInputBuffer();
                    printf("[ERROR] Invalid input.\n");
                } else {
                    clearInputBuffer();
                    displayTopProductsByCategory(manager, n);
                }
                clearScreen();
                break;

            case 5:
                displayAllCategories(manager);
                clearScreen();
                break;

            case 6:
                printf("\nEnter category to search: ");
                fgets(category, MAX_CATEGORY_LEN, stdin);
                category[strcspn(category, "\n")] = 0;
                searchByCategory(manager, category);
                clearScreen();
                break;

            case 7:
                printf("\nEnter product name or keyword: ");
                fgets(keyword, MAX_NAME_LEN, stdin);
                keyword[strcspn(keyword, "\n")] = 0;
                searchByName(manager, keyword);
                clearScreen();
                break;

            case 8:
                displayCategoryStatistics(manager);
                clearScreen();
                break;

            case 9:
                displayGlobalStatistics(manager);
                clearScreen();
                break;

            case 10:
                displayCategoryWeights(manager);
                clearScreen();
                break;

            case 11:
                customizeWeights(manager);
                clearScreen();
                break;

            case 12:
                printf("\nEnter filename (e.g., products.csv): ");
                fgets(filename, 100, stdin);
                filename[strcspn(filename, "\n")] = 0;
                exportToCSV(manager, filename);
                clearScreen();
                break;

            case 13:
                printf("\n[INFO] Loading sample products...\n");
                addSampleProducts(manager);
                printf("[SUCCESS] Sample products loaded! Categories: %d\n", manager->num_categories);
                clearScreen();
                break;

            case 0:
                printf("\n");
                printLine(100);
                printf("              Thank you for using the Category-Aware Ranking System!\n");
                printf("                    Products ranked fairly within categories!\n");
                printLine(100);
                printf("\n");
                freeCategoryManager(manager);
                return 0;

            default:
                printf("\n[ERROR] Invalid choice! Please try again.\n");
                clearScreen();
        }
    }

    return 0;
}

